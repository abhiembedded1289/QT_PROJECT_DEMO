#include <QApplication>
#include <QPushButton>
#include <QMessageBox>
#include <QWidget>
#include <QVBoxLayout>

int main(int argc, char *argv[])
{
    // QApplication manages the GUI and receives keyboard and mouse events.
    QApplication app(argc, argv);

    // QWidget is the main window that contains our buttons.
    QWidget window;
    window.setWindowTitle("Button Click Practice");

    // A button emits the clicked signal when the user presses it.
    QPushButton *button = new QPushButton("Click Me");
    
    // Connect the signal to code that shows feedback to the user.
    QObject::connect(button, &QPushButton::clicked, [&window](){
        QMessageBox::information(&window, "Button Clicked",
                                 "<span style='color:red; font-size:32px;'>&#9829;</span>");
    });
    
    QPushButton *exitButton = new QPushButton("Exit");

    // A second connection can perform another action for the same click.
    QObject::connect(button, &QPushButton::clicked, [](){
        qDebug("Button was clicked!");
    });

    // Connect Exit directly to Qt's built-in application shutdown function.
    QObject::connect(exitButton, &QPushButton::clicked, &app, &QApplication::quit);

    // A vertical layout keeps the buttons arranged from top to bottom.
    QVBoxLayout *layout = new QVBoxLayout;
    layout->addWidget(button);
    layout->addWidget(exitButton);
    window.setLayout(layout);

    window.resize(300, 180);
    window.show();

    // Start the event loop so the window stays open and reacts to events.
    return app.exec();
}
