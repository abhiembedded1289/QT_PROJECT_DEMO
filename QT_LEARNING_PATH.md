# Qt Beginner Learning Path

Use one project at a time. For every change:

1. Read the code comments.
2. Change one small thing.
3. Save with `Ctrl+S`.
4. Build with `Ctrl+Shift+B`.
5. Run with `F5`.
6. Write down what changed.

## Project 1: Hello Qt

Folder: `01_hello_qt`

Topics:

- `QApplication`
- `QLabel`
- Window size and alignment
- The Qt event loop

Practice:

- Change the label text.
- Change the window size.
- Change the alignment.

Why it matters: every Qt application needs an application object, visible widgets, and an event loop that waits for user actions.

## Project 2: Button Click

Folder: `02_button_click`

Topics:

- `QPushButton`
- Signals and slots
- Lambda functions
- `QMessageBox`
- `QVBoxLayout`

Practice:

- Change the button text.
- Show a different message.
- Add a second message button.
- Make the Exit button show a confirmation first.

Why it matters: production applications react to user actions in this way. A click can save data, open a page, start audio, or read a meter.

## Project 3: Calculator

Folder: `03_calculator`

Topics:

- `QLineEdit`
- Reading user input
- Converting text to numbers
- Validating input
- Displaying results

Practice:

- Add subtraction.
- Reject empty input.
- Show an error for invalid text.
- Add a Clear button.

Why it matters: real applications receive user input, validate it, perform business logic, and display a result.

## Project 4: Smart Meter Screen

Create: `04_smart_meter`

Build a screen with voltage, current, power, and a Refresh button. Start with fake values such as `230 V` and `5 A`.

Topics:

- Multiple labels
- Layouts
- A controller function
- Updating widgets
- `QTimer`

Why it matters: begin with fake data so you can learn the UI before connecting real hardware.

## Project 5: Soundbox Controller

Create: `05_soundbox`

Build Play, Pause, Stop, and Volume controls. Start by changing labels when buttons are clicked. Later add `QMediaPlayer`.

Topics:

- Application state
- Sliders
- Button state
- Qt Multimedia

Why it matters: a soundbox combines user actions, device state, and feedback on the screen.

## Project 6: Serial Meter Reader

Create: `06_serial_meter`

Read a line of test data from a serial port and display it on screen.

Topics:

- `QSerialPort`
- Signals for incoming data
- Parsing text
- Error handling

Why it matters: this is the bridge from a practice UI to a real smart meter device.

## Project 7: Settings Window

Create: `07_settings`

Let the user save a device name, port name, and update interval.

Topics:

- Separate windows
- `QSettings`
- Form layouts
- Persistent data

Why it matters: production applications remember user preferences between runs.

## Project 8: Production-style Dashboard

Create: `08_dashboard`

Combine a meter display, connection status, history table, settings, and error messages.

Topics:

- Multiple classes
- Model/view design
- Keeping UI separate from device logic
- Testing

Why it matters: larger applications stay understandable when the user interface, business logic, and device communication are separate.

## Vocabulary

- Widget: a visible UI object such as a label or button.
- Signal: a notification that something happened.
- Slot: code that responds to a signal.
- Layout: a rule that arranges widgets.
- Event loop: Qt's loop that waits for clicks, keys, timers, and other events.
- Build: compile and link C++ code into an executable.
- Debug: run with breakpoints and inspect values.

Do not try to memorize everything. Change one line, run the program, and explain the result in your own words.
