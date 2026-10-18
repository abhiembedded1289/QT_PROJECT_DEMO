#include <QApplication> // Provides the Qt application and event loop.
#include <QLabel>       // Provides a widget that displays text.

int main(int argc, char *argv[])
{
    // QApplication manages the GUI and handles mouse and keyboard events.
    QApplication app(argc, argv);

    // QLabel creates a simple window that displays this text.
    QLabel label("Hello abhi, Qt Practice!");

    // Set the window size in pixels: width 260, height 80.
    label.resize(260, 80);

    // Put the text in the center of the label.
    label.setAlignment(Qt::AlignCenter);

    // Make the label window visible on the screen.
    label.show();

    // Start the event loop so the window stays open and responds to events.
    return app.exec();
}
