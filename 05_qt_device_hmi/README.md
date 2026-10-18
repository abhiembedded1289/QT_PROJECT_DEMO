# Qt HMI and Embedded Device TCP Demo

This standalone learning project contains two programs:

- `smart_hmi`: a Qt Widgets interface with temperature, battery, LED, connection status, and LED/start/stop controls.
- `device_server`: a C TCP server that simulates an embedded device. Its code separates sensor sampling, device state, command handling, and TCP transport.

The device currently uses simulated readings and listens only on `127.0.0.1:45454`. Replace the sensor simulation and loopback address when adapting it to real hardware. The PC-hosted C server is a protocol simulator, not firmware for a specific microcontroller.

## Build

Run these commands from the `qt-practices` workspace root in PowerShell:

```powershell
$env:PATH = 'C:\Qt\Tools\mingw1310_64\bin;C:\Qt\Tools\Ninja;C:\Qt\6.11.2\mingw_64\bin;' + $env:PATH
& 'C:\Qt\Tools\CMake_64\bin\cmake.exe' -S '.\05_qt_device_hmi' -B '.\05_qt_device_hmi\build' -G Ninja -DCMAKE_PREFIX_PATH='C:\Qt\6.11.2\mingw_64'
if ($LASTEXITCODE -eq 0) { & 'C:\Qt\Tools\CMake_64\bin\cmake.exe' --build '.\05_qt_device_hmi\build' }
```

## Run

Open two PowerShell terminals at the workspace root. Start the device server first:

```powershell
& '.\05_qt_device_hmi\build\device_server.exe'
```

Then start the Qt HMI in the second terminal:

```powershell
$env:PATH = 'C:\Qt\6.11.2\mingw_64\bin;' + $env:PATH
& '.\05_qt_device_hmi\build\smart_hmi.exe'
```

The HMI retries the connection while the server is offline. Once connected, use `LED ON` / `LED OFF` to control the simulated LED, and `START` / `STOP` to start or stop periodic sensor updates. Press `Ctrl+C` in the server terminal to stop the device simulator.

## TCP protocol

Commands are UTF-8/ASCII text terminated by a newline:

```text
LED ON
LED OFF
START
STOP
```

The server replies with a state line after connecting and after each valid command. While acquisition is running, it sends a new state about once per second:

```text
STATE|temperature_c|battery_percent|led_on|streaming
```

For example, `STATE|32.5|78|0|0` means 32.5 degrees C, 78 percent battery, LED off, and sensor streaming stopped. Keeping the protocol line-based makes it easy to inspect with a terminal or replace the simulator with an embedded TCP server.