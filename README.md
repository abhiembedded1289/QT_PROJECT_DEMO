# Qt Practice Projects

This folder is for creating many small Qt and C++ learning projects.

## Structure

- `01_hello_qt` - your first Qt window app
- Add new folders like `02_button_click`, `03_calculator`, etc.

## Open in VS Code

1. Open this folder in VS Code.
2. Open a project folder such as `01_hello_qt`.
3. Use the integrated terminal to build and run it.

## Build a project

```bash
cmake -S . -B build
cmake --build build
```

## Run a project

On Windows:

```bash
./build/Debug/qt_practice.exe
```

or if the build is in a different folder:

```bash
./build/qt_practice.exe
```

## Use in VS Code

Open the `qt-practices` folder as the workspace. The included `.vscode` setup is configured for the installed Qt 6.11.2 MinGW kit.

- Press `Ctrl+Shift+B` to build `01_hello_qt`.
- Press `F5` to build and run it with the debugger.
- Use `Ctrl+Shift+P` and choose `Tasks: Run Task` to run the configure task if the build folder is missing.

The Qt window opens with the text `Hello, Qt Practice!`.

## Create a new project quickly

Use the script:

```powershell
cd qt-practices
./new_project.ps1 02_button_click
```

This creates a new folder with a basic Qt app template.

## Learn step by step

Read [QT_LEARNING_PATH.md](QT_LEARNING_PATH.md) for the beginner lessons, practice exercises, and the reason each Qt topic is useful in a real application.

1. Start with `01_hello_qt`
2. Learn QLabel, QPushButton, QLineEdit
3. Practice layouts and signals/slots
4. Build small projects like calculator, login form, and timer

## Notes

- You may need Qt installed on your machine.
- If Qt is not installed, install Qt 6 or Qt 5 before building.
- Use CMake from VS Code with the C++ extension for better auto-completion and build support.
