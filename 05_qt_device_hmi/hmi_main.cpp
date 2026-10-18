#include <QApplication> // Owns application-wide GUI resources and the event loop.
#include <QByteArray> // Stores the raw bytes sent to and received from the TCP server.
#include <QDateTime> // Adds a time stamp to each activity-log entry.
#include <QDebug> // Provides Qt diagnostic output while developing.
#include <QFrame> // Provides visual panels around connection and measurement content.
#include <QGridLayout> // Arranges the four measurement panels in two rows and columns.
#include <QHBoxLayout> // Arranges status text and button groups from left to right.
#include <QLabel> // Displays headings, connection state, readings, and status messages.
#include <QPlainTextEdit> // Displays a scrollable, read-only activity log.
#include <QPushButton> // Provides controls that send commands to the device.
#include <QTcpSocket> // Connects this HMI client to the device's TCP server.
#include <QTimer> // Retries the TCP connection while the device is offline.
#include <QVBoxLayout> // Stacks the main dashboard sections vertically.
#include <QWidget> // Provides the base class for the HMI window.

namespace {

// Build one reusable measurement panel and return its value label to the caller.
QFrame *createMetric(const QString &name, const QString &unit, QLabel **valueLabel)
{
    auto *panel = new QFrame; // The frame gives one reading a distinct panel.
    auto *layout = new QVBoxLayout(panel); // Stack the name, changing value, and unit vertically.
    layout->setContentsMargins(16, 13, 16, 14); // Keep the text away from the panel edges.
    layout->setSpacing(4); // Keep each part close while retaining visual separation.

    auto *nameLabel = new QLabel(name); // Shows which device measurement this panel represents.
    nameLabel->setAlignment(Qt::AlignCenter); // Centers the measurement name within the panel.
    *valueLabel = new QLabel("--"); // Starts with a placeholder until a device message arrives.
    (*valueLabel)->setAlignment(Qt::AlignCenter); // Centers the changing reading.
    auto *unitLabel = new QLabel(unit); // Tells the user the unit or state represented by the value.
    unitLabel->setAlignment(Qt::AlignCenter); // Aligns the unit with its reading.

    layout->addWidget(nameLabel); // Put the measurement name at the top of the panel.
    layout->addWidget(*valueLabel); // Put the live value in the main visual position.
    layout->addWidget(unitLabel); // Put its unit below the value.
    return panel; // Give the completed panel to the dashboard layout.
}

} // namespace

class SmartHmi : public QWidget
{
public:
    SmartHmi()
    {
        setWindowTitle("Device HMI | Smart Monitor"); // Name the window in its title bar.
        setMinimumSize(580, 600); // Prevent the controls and four readings from being clipped.
        resize(640, 650); // Choose a comfortable initial size for the whole dashboard.

        auto *page = new QVBoxLayout(this); // Stack all dashboard sections inside this window.
        page->setContentsMargins(26, 24, 26, 22); // Add breathing room around the window contents.
        page->setSpacing(12); // Keep a consistent gap between dashboard sections.

        auto *heading = new QLabel("Device Monitor"); // Identifies the purpose of this HMI screen.
        auto *subtitle = new QLabel("LIVE EMBEDDED TELEMETRY"); // Indicates that readings come from the device connection.
        page->addWidget(heading); // Place the main heading first.
        page->addWidget(subtitle); // Place the telemetry description below the heading.

        auto *connectionPanel = new QFrame; // Visually groups the network connection indicator.
        auto *connectionLayout = new QHBoxLayout(connectionPanel); // Put the label and status on one row.
        connectionLayout->setContentsMargins(14, 10, 14, 10); // Inset both items from the panel border.
        auto *connectionTitle = new QLabel("TCP DEVICE"); // Names the connection being monitored.
        deviceValue = new QLabel("OFFLINE"); // Shows OFFLINE until the TCP socket connects.
        connectionLayout->addWidget(connectionTitle); // Keep the connection name at the left.
        connectionLayout->addStretch(); // Push the changing connection state to the right.
        connectionLayout->addWidget(deviceValue); // Display ONLINE or OFFLINE at the right.
        page->addWidget(connectionPanel); // Place connection status above the readings.

        auto *metrics = new QGridLayout; // A grid keeps all four readings aligned in two rows.
        metrics->setSpacing(12); // Add space between neighboring measurement panels.
        metrics->addWidget(createMetric("Temperature", "degrees C", &temperatureValue), 0, 0); // Save the temperature value label for TCP updates.
        metrics->addWidget(createMetric("Battery", "percent", &batteryValue), 0, 1); // Save the battery value label for TCP updates.
        metrics->addWidget(createMetric("LED", "device output", &ledValue), 1, 0); // Show the device's current LED state.
        metrics->addWidget(createMetric("Acquisition", "device state", &runValue), 1, 1); // Show whether sensor streaming is running.
        page->addLayout(metrics); // Add the readings between connection status and controls.
        page->setStretch(3, 1); // Let the metric area absorb extra height so it stays easy to read.

        auto *ledButtons = new QHBoxLayout; // Keep the two LED commands together on one row.
        auto *ledLabel = new QLabel("LED CONTROL"); // Distinguish LED commands from stream controls.
        ledOnButton = new QPushButton("LED ON"); // Create the command that turns the device LED on.
        ledOffButton = new QPushButton("LED OFF"); // Create the command that turns the device LED off.
        ledButtons->addWidget(ledLabel); // Label this row at its left edge.
        ledButtons->addStretch(); // Keep the buttons aligned at the right side.
        ledButtons->addWidget(ledOnButton); // Add the LED-on action.
        ledButtons->addWidget(ledOffButton); // Add the LED-off action beside it.
        page->addLayout(ledButtons); // Place LED controls below the readings.

        auto *runButtons = new QHBoxLayout; // Keep stream start and stop actions together.
        auto *runLabel = new QLabel("SENSOR STREAM"); // Explains that these buttons control data acquisition.
        startButton = new QPushButton("START"); // Create the command that begins periodic sampling.
        stopButton = new QPushButton("STOP"); // Create the command that stops periodic sampling.
        runButtons->addWidget(runLabel); // Identify this group of controls.
        runButtons->addStretch(); // Align stream buttons with the LED buttons above.
        runButtons->addWidget(startButton); // Add the start action.
        runButtons->addWidget(stopButton); // Add the stop action.
        page->addLayout(runButtons); // Place stream controls below LED controls.

        message = new QLabel("Start device_server, then connecting automatically..."); // Shows connection and protocol feedback.
        page->addWidget(message); // Keep the latest status near the controls.

        auto *logTitle = new QLabel("ACTIVITY LOG"); // Names the event history shown below.
        page->addWidget(logTitle); // Place the log heading immediately above the log.
        activityLog = new QPlainTextEdit; // Stores timestamped connection, command, and state events.
        activityLog->setReadOnly(true); // Prevent user edits to diagnostic output.
        activityLog->setMaximumBlockCount(100); // Bound memory use by keeping only recent events.
        activityLog->setFixedHeight(74); // Reserve a compact area for recent log lines.
        page->addWidget(activityLog); // Add the activity history at the bottom of the window.

        setControlsEnabled(false); // Prevent commands until the device connection is established.
        logMessage("HMI started"); // Record startup so the activity log has a first entry.

        // Send one newline-terminated command; TCP may split or combine packets,
        // so the device server treats newline as the command boundary.
        connect(ledOnButton, &QPushButton::clicked, this, [this]() { sendCommand("LED ON"); }); // Turn on the device LED after a click.
        connect(ledOffButton, &QPushButton::clicked, this, [this]() { sendCommand("LED OFF"); }); // Turn off the device LED after a click.
        connect(startButton, &QPushButton::clicked, this, [this]() { sendCommand("START"); }); // Ask the device to begin sending periodic readings.
        connect(stopButton, &QPushButton::clicked, this, [this]() { sendCommand("STOP"); }); // Ask the device to stop periodic readings.

        // Update the UI as soon as the TCP handshake succeeds.
        connect(&socket, &QTcpSocket::connected, this, [this]() {
            deviceValue->setText("ONLINE");
            message->setText("Connected to 127.0.0.1:45454");
            setControlsEnabled(true);
            logMessage("Connected to device at 127.0.0.1:45454");
        });

        // Disable controls if the server closes the connection unexpectedly.
        connect(&socket, &QTcpSocket::disconnected, this, [this]() {
            showOffline("Device disconnected; retrying...");
            logMessage("Device disconnected");
        });

        // Explain connection failures and wait for the retry timer to reconnect.
        connect(&socket, &QTcpSocket::errorOccurred, this, [this](QAbstractSocket::SocketError) {
            if (socket.state() == QAbstractSocket::UnconnectedState) {
                showOffline("Device offline; retrying connection...");
                logMessage("TCP error: " + socket.errorString());
            }
        });

        // Process every complete protocol line; one TCP read may contain partial or multiple lines.
        connect(&socket, &QTcpSocket::readyRead, this, [this]() {
            // Read complete lines only; TCP is a byte stream, not a message queue.
            while (socket.canReadLine())
                handleDeviceLine(socket.readLine().trimmed());
        });

        reconnectTimer.setInterval(1500); // Wait briefly between connection attempts instead of busy-looping.
        connect(&reconnectTimer, &QTimer::timeout, this, [this]() { connectToDevice(); }); // Retry when each interval expires.
        reconnectTimer.start(); // Keep retrying until the device server becomes available.
        connectToDevice(); // Try immediately instead of waiting for the first timer tick.
    }

private:
    void connectToDevice()
    {
        if (socket.state() == QAbstractSocket::UnconnectedState) {
            logMessage("Connecting to device at 127.0.0.1:45454"); // Record each new connection attempt.
            socket.connectToHost("127.0.0.1", 45454); // Start an asynchronous TCP connection to the simulator.
        }
    }

    void sendCommand(const QByteArray &command)
    {
        if (socket.state() != QAbstractSocket::ConnectedState) {
            logMessage("Command not sent; device is offline: " + QString::fromUtf8(command)); // Explain why the action could not be sent.
            return; // Do not write to a socket that is not connected.
        }

        const QByteArray message = command + '\n'; // Newline marks the end of one command in the text protocol.
        if (socket.write(message) == message.size()) {
            socket.flush(); // Push queued command bytes toward the device promptly.
            logMessage("Sent command: " + QString::fromUtf8(command)); // Show the exact command in the HMI log.
        } else {
            logMessage("Failed to queue command: " + QString::fromUtf8(command)); // Report a local socket-write problem.
        }
    }

    void handleDeviceLine(const QByteArray &line)
    {
        // Protocol: STATE|temperature|battery|led(0/1)|streaming(0/1)
        const QList<QByteArray> fields = line.split('|'); // Split the state message into its five pipe-delimited fields.
        if (fields.size() != 5 || fields[0] != "STATE") {
            message->setText("Received an invalid device message"); // Tell the user the message did not match the protocol.
            return; // Do not update readings from an unrecognized message.
        }

        bool temperatureOk = false; // Receives whether the temperature text converted successfully.
        bool batteryOk = false; // Receives whether the battery text converted successfully.
        const double temperature = fields[1].toDouble(&temperatureOk); // Parse the temperature measurement.
        const int battery = fields[2].toInt(&batteryOk); // Parse the battery percentage as an integer.
        if (!temperatureOk || !batteryOk) {
            message->setText("Received invalid sensor values"); // Report numeric data that could not be parsed.
            return; // Keep the last valid readings on screen.
        }

        temperatureValue->setText(QString::number(temperature, 'f', 1)); // Show temperature with one decimal place.
        batteryValue->setText(QString::number(battery) + "%"); // Show battery percentage with its unit.
        ledValue->setText(fields[3] == "1" ? "ON" : "OFF"); // Convert the protocol bit into readable LED state.
        runValue->setText(fields[4] == "1" ? "RUNNING" : "STOPPED"); // Convert the stream bit into readable state.
        message->setText("Device data received over TCP"); // Confirm that a valid state update arrived.
        // Log the same values shown on screen so connection behavior can be diagnosed later.
        logMessage(QString("Device state: %1 C, battery %2, LED %3, stream %4")
                       .arg(temperatureValue->text(), batteryValue->text(),
                            ledValue->text(), runValue->text()));
    }

    void logMessage(const QString &text)
    {
        const QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss"); // Give each event an easy-to-read time.
        activityLog->appendPlainText(timestamp + "  " + text); // Add the event at the bottom of the visible log.
    }

    void setControlsEnabled(bool enabled)
    {
        ledOnButton->setEnabled(enabled); // Only allow LED commands when a device can receive them.
        ledOffButton->setEnabled(enabled); // Keep both LED controls in the same connection state.
        startButton->setEnabled(enabled); // Only allow streaming commands while connected.
        stopButton->setEnabled(enabled); // Keep the stop control consistent with the other commands.
    }

    void showOffline(const QString &status)
    {
        deviceValue->setText("OFFLINE"); // Make the current connection state explicit.
        message->setText(status); // Explain what the HMI is doing about the connection.
        setControlsEnabled(false); // Block commands until the server reconnects.
    }

    QTcpSocket socket; // Connects to the device server to send controls and receive live readings.
    QTimer reconnectTimer; // Schedules another connection attempt if the device is offline.
    QLabel *temperatureValue = nullptr; // Points to the temperature value label created in its metric panel.
    QLabel *batteryValue = nullptr; // Points to the battery value label created in its metric panel.
    QLabel *ledValue = nullptr; // Points to the label showing whether the device LED is on.
    QLabel *runValue = nullptr; // Points to the label showing whether sensor streaming is active.
    QLabel *deviceValue = nullptr; // Points to the online/offline connection indicator.
    QLabel *message = nullptr; // Points to the latest human-readable connection or data status.
    QPlainTextEdit *activityLog = nullptr; // Points to the read-only event history widget.
    QPushButton *ledOnButton = nullptr; // Points to the LED-on command button.
    QPushButton *ledOffButton = nullptr; // Points to the LED-off command button.
    QPushButton *startButton = nullptr; // Points to the sensor-stream start command button.
    QPushButton *stopButton = nullptr; // Points to the sensor-stream stop command button.
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv); // Set up Qt before creating any GUI widgets.
    SmartHmi window; // Construct the dashboard and its TCP connection logic.
    window.show(); // Make the main window visible to the user.
    return app.exec(); // Run Qt's event loop so the UI and socket can process events.
}