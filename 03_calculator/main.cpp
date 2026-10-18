#include <QApplication>
#include <QGridLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QWidget>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    QWidget window;
    window.setWindowTitle("Calculator Practice");

    QLineEdit *display = new QLineEdit("0");
    display->setReadOnly(true);
    display->setAlignment(Qt::AlignRight);

    QStringList labels = {"7", "8", "9", "/",
                          "4", "5", "6", "*",
                          "1", "2", "3", "-",
                          "0", "C", "=", "+"};

    QGridLayout *layout = new QGridLayout;
    layout->addWidget(display, 0, 0, 1, 4);

    int index = 0;
    for (const QString &text : labels) {
        QPushButton *button = new QPushButton(text);
        layout->addWidget(button, 1 + (index / 4), index % 4);
        if (text == "C") {
            QObject::connect(button, &QPushButton::clicked, [display]() {
                display->setText("0");
            });
        }
        ++index;
    }

    window.setLayout(layout);
    window.resize(300, 400);
    window.show();

    return app.exec();
}
