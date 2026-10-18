#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <winsock2.h>
#include <ws2tcpip.h>
typedef SOCKET DeviceSocket;
#define CLOSE_SOCKET closesocket
#else
#include <arpa/inet.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <unistd.h>
typedef int DeviceSocket;
#define CLOSE_SOCKET close
#endif

#define DEVICE_PORT 45454
#define COMMAND_BUFFER_SIZE 128

typedef struct {
    float temperature;
    int battery_percent;
    int led_on;
    int streaming;
} DeviceState;

static volatile sig_atomic_t server_running = 1;

static void stop_server(int signal_number)
{
    (void)signal_number;
    server_running = 0;
}

// Sensor Manager: replace this simulator with ADC/I2C/SPI sensor reads on hardware.
static void sensor_manager_sample(DeviceState *state)
{
    state->temperature += 0.1f;
    if (state->temperature > 35.0f)
        state->temperature = 32.0f;

    if (state->battery_percent > 0)
        --state->battery_percent;
    else
        state->battery_percent = 100;
}

// Device Manager: owns device state transitions independently of TCP transport.
static void device_manager_start(DeviceState *state)
{
    state->streaming = 1;
}

static void device_manager_stop(DeviceState *state)
{
    state->streaming = 0;
}

// Command Handler: translate simple network commands into device state changes.
static int command_handler_process(const char *command, DeviceState *state)
{
    if (strcmp(command, "LED ON") == 0) {
        state->led_on = 1;
    } else if (strcmp(command, "LED OFF") == 0) {
        state->led_on = 0;
    } else if (strcmp(command, "START") == 0) {
        device_manager_start(state);
    } else if (strcmp(command, "STOP") == 0) {
        device_manager_stop(state);
    } else {
        printf("Unknown command: %s\n", command);
        return 0;
    }

    printf("Command handled: %s\n", command);
    return 1;
}

static int send_all(DeviceSocket client, const char *data, size_t length)
{
    size_t sent = 0;
    while (sent < length) {
        const int result = send(client, data + sent, (int)(length - sent), 0);
        if (result <= 0)
            return 0;
        sent += (size_t)result;
    }
    return 1;
}

static int tcp_server_send_state(DeviceSocket client, const DeviceState *state)
{
    char message[96];
    const int length = snprintf(message, sizeof(message), "STATE|%.1f|%d|%d|%d\n",
                                state->temperature, state->battery_percent,
                                state->led_on, state->streaming);
    return length > 0 && (size_t)length < sizeof(message)
        && send_all(client, message, (size_t)length);
}

static int serve_client(DeviceSocket client)
{
    DeviceState state = {32.5f, 78, 0, 0};
    char command[COMMAND_BUFFER_SIZE];
    size_t command_length = 0;
    int discard_long_command = 0;
    time_t last_sample = time(NULL);

    if (!tcp_server_send_state(client, &state))
        return 0;

    while (server_running) {
        fd_set readable;
        FD_ZERO(&readable);
        FD_SET(client, &readable);
        struct timeval timeout = {0, 200000};

#ifdef _WIN32
        const int ready = select(0, &readable, NULL, NULL, &timeout);
#else
        const int ready = select(client + 1, &readable, NULL, NULL, &timeout);
#endif
        if (ready < 0)
            break;

        if (ready > 0 && FD_ISSET(client, &readable)) {
            char incoming[256];
            const int received = recv(client, incoming, (int)sizeof(incoming), 0);
            if (received <= 0)
                break;

            for (int index = 0; index < received; ++index) {
                const char character = incoming[index];
                if (character == '\n') {
                    if (!discard_long_command) {
                        command[command_length] = '\0';
                        if (command_length > 0 && command[command_length - 1] == '\r')
                            command[command_length - 1] = '\0';
                        if (command_handler_process(command, &state)
                            && !tcp_server_send_state(client, &state))
                            return 0;
                    }
                    command_length = 0;
                    discard_long_command = 0;
                } else if (!discard_long_command) {
                    if (command_length + 1 < sizeof(command))
                        command[command_length++] = character;
                    else {
                        command_length = 0;
                        discard_long_command = 1;
                    }
                }
            }
        }

        const time_t now = time(NULL);
        if (state.streaming && difftime(now, last_sample) >= 1.0) {
            sensor_manager_sample(&state);
            if (!tcp_server_send_state(client, &state))
                break;
            last_sample = now;
        }
    }

    return 1;
}

// TCP Server: accept a HMI client and pass newline-delimited commands/state.
int main(void)
{
#ifdef _WIN32
    WSADATA winsock_data;
    if (WSAStartup(MAKEWORD(2, 2), &winsock_data) != 0) {
        fprintf(stderr, "Could not initialize Winsock.\n");
        return EXIT_FAILURE;
    }
#endif

    signal(SIGINT, stop_server);

    const DeviceSocket listener = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
#ifdef _WIN32
    if (listener == INVALID_SOCKET) {
#else
    if (listener < 0) {
#endif
        fprintf(stderr, "Could not create TCP socket.\n");
#ifdef _WIN32
        WSACleanup();
#endif
        return EXIT_FAILURE;
    }

    int reuse_address = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR,
#ifdef _WIN32
               (const char *)&reuse_address,
#else
               &reuse_address,
#endif
               (int)sizeof(reuse_address));

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_port = htons(DEVICE_PORT);
    address.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    if (bind(listener, (struct sockaddr *)&address, (int)sizeof(address)) != 0
        || listen(listener, 1) != 0) {
        fprintf(stderr, "Could not listen on 127.0.0.1:%d. Is the port already in use?\n",
                DEVICE_PORT);
        CLOSE_SOCKET(listener);
#ifdef _WIN32
        WSACleanup();
#endif
        return EXIT_FAILURE;
    }

    printf("Embedded device simulator listening on 127.0.0.1:%d\n", DEVICE_PORT);
    printf("Start the Qt HMI, then use its LED and sensor-stream controls.\n");

    while (server_running) {
        const DeviceSocket client = accept(listener, NULL, NULL);
#ifdef _WIN32
        if (client == INVALID_SOCKET) {
#else
        if (client < 0) {
#endif
            if (server_running)
                fprintf(stderr, "Could not accept the HMI connection.\n");
            continue;
        }

        printf("HMI connected.\n");
        (void)serve_client(client);
#ifdef _WIN32
        shutdown(client, SD_BOTH);
#else
        shutdown(client, SHUT_RDWR);
#endif
        CLOSE_SOCKET(client);
        printf("HMI disconnected. Waiting for another connection.\n");
    }

    CLOSE_SOCKET(listener);
#ifdef _WIN32
    WSACleanup();
#endif
    printf("Device server stopped.\n");
    return EXIT_SUCCESS;
}