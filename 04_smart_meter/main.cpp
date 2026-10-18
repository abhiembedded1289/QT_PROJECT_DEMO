#include <QApplication>
#include <QWidget>
#include <QLabel>
#include <QPushButton>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QTimer>
#include <QObject>
#include <QStyle>

// --------------------------------------------------
// Structure to store all meter values
// --------------------------------------------------
struct MeterData
{
    float voltage;
    float current;
    float power;
    float energy;
};


// --------------------------------------------------
// MeterManager
// Responsible for getting meter data
// --------------------------------------------------
class MeterManager : public QObject
{
    Q_OBJECT

public:

    MeterManager(QObject *parent = nullptr)
        : QObject(parent)
    {
    }

    // Simulate reading data from smart meter
    void readMeter()
    {
        static float voltage = 230.0f;
        static float current = 5.0f;
        static float energy  = 1245.0f;

        // Simulate changing values
        voltage += 0.2f;
        current += 0.05f;

        // Calculate power
        float power = (voltage * current) / 1000.0f;

        // Simulate energy increase
        energy += 0.01f;

        // Store values in structure
        MeterData data;

        data.voltage = voltage;
        data.current = current;
        data.power   = power;
        data.energy  = energy;

        // Tell MainWindow that new data is available
        emit meterDataReady(data);
    }

signals:

    // Signal sent whenever new meter data is available
    void meterDataReady(MeterData data);
};


// --------------------------------------------------
// MainWindow
// Responsible for displaying the HMI
// --------------------------------------------------
class MainWindow : public QWidget
{
    Q_OBJECT

public:

    MainWindow()
    {
        // ==================================================
        // 1. Create title
        // ==================================================

        title = new QLabel("SMART METER");
        title->setObjectName("title");
        title->setAlignment(Qt::AlignCenter);


        // ==================================================
        // 2. Create meter value labels
        // ==================================================

        voltage = new QLabel("230.0 V");
        current = new QLabel("5.0 A");
        power   = new QLabel("1.15 kW");
        energy  = new QLabel("1245.0 kWh");
        voltage->setObjectName("voltageValue");
        current->setObjectName("currentValue");
        power->setObjectName("powerValue");
        energy->setObjectName("energyValue");


        // ==================================================
        // 3. Create labels for parameter names
        // ==================================================

        voltageName = new QLabel("Voltage");
        currentName = new QLabel("Current");
        powerName   = new QLabel("Power");
        energyName  = new QLabel("Energy");
        voltageName->setObjectName("metricName");
        currentName->setObjectName("metricName");
        powerName->setObjectName("metricName");
        energyName->setObjectName("metricName");


        // ==================================================
        // 4. Create main Meter Data box
        // ==================================================

        QGroupBox *meterBox = new QGroupBox("Meter Data");
        meterBox->setObjectName("meterBox");
        meterBox->setAlignment(Qt::AlignCenter);


        // ==================================================
        // 5. Create grid layout for meter parameters
        // ==================================================

        QGridLayout *meterLayout = new QGridLayout;


        // First row
        meterLayout->addWidget(voltageName, 0, 0);
        meterLayout->addWidget(voltage,     0, 1);

        meterLayout->addWidget(currentName, 0, 2);
        meterLayout->addWidget(current,     0, 3);


        // Second row
        meterLayout->addWidget(powerName,  1, 0);
        meterLayout->addWidget(power,      1, 1);

        meterLayout->addWidget(energyName, 1, 2);
        meterLayout->addWidget(energy,     1, 3);


        // Put grid layout inside Meter Data box
        meterBox->setLayout(meterLayout);


        // ==================================================
        // 6. Create Monitoring button
        // ==================================================

        monitorButton = new QPushButton("Monitoring: OFF");
        monitorButton->setObjectName("monitorButton");
        monitorButton->setProperty("monitoring", false);


        // ==================================================
        // 7. Create timer
        // ==================================================

        timer = new QTimer(this);


        // ==================================================
        // 8. Create MeterManager
        // ==================================================

        meter = new MeterManager(this);


        // ==================================================
        // 9. Main window layout
        // ==================================================

        QVBoxLayout *mainLayout = new QVBoxLayout;
        mainLayout->setContentsMargins(24, 22, 24, 22);
        mainLayout->setSpacing(16);


        // Add title
        mainLayout->addWidget(title);


        // Add complete Meter Data box
        mainLayout->addWidget(meterBox);


        // Add Monitoring button OUTSIDE the box
        mainLayout->addWidget(monitorButton);


        // Apply layout to MainWindow
        setLayout(mainLayout);

        setStyleSheet(
            "QWidget { background: #edf3ef; color: #203a30; font-family: 'Segoe UI'; }"
            "QLabel#title { background: #164b3a; color: #ffffff; border-radius: 8px;"
            " font-size: 22px; font-weight: 700; padding: 14px; }"
            "QGroupBox#meterBox { background: #ffffff; border: 1px solid #cbdcd1;"
            " border-radius: 8px; margin-top: 14px; padding: 18px 12px 12px;"
            " color: #365a49; font-size: 15px; font-weight: 700; }"
            "QGroupBox#meterBox::title { subcontrol-origin: margin;"
            " subcontrol-position: top center; padding: 0 9px; background: #ffffff; }"
            "QLabel#metricName { color: #687a70; font-size: 13px; }"
            "QLabel#voltageValue { color: #23734f; font-size: 17px; font-weight: 700; }"
            "QLabel#currentValue { color: #28749a; font-size: 17px; font-weight: 700; }"
            "QLabel#powerValue { color: #bd761d; font-size: 17px; font-weight: 700; }"
            "QLabel#energyValue { color: #665b91; font-size: 17px; font-weight: 700; }"
            "QPushButton#monitorButton { background: #23734f; color: #ffffff;"
            " border: 0; border-radius: 6px; padding: 11px; font-size: 14px; font-weight: 700; }"
            "QPushButton#monitorButton:hover { background: #195d3e; }"
            "QPushButton#monitorButton[monitoring='true'] { background: #b94e3c; }"
            "QPushButton#monitorButton[monitoring='true']:hover { background: #9f3c2e; }");


        // Set window size
        resize(500, 300);


        // ==================================================
        // 10. Timer -> updateMeter()
        // ==================================================

        connect(timer,
                &QTimer::timeout,
                this,
                &MainWindow::updateMeter);


        // ==================================================
        // 11. Monitoring button
        // ==================================================

        connect(monitorButton,
                &QPushButton::clicked,
                this,
                &MainWindow::toggleMonitoring);


        // ==================================================
        // 12. MeterManager -> MainWindow
        // ==================================================

        connect(meter,
                &MeterManager::meterDataReady,
                this,
                &MainWindow::updateDisplay);
    }


private:


    // ==================================================
    // Called every 1 second when monitoring is ON
    // ==================================================

    void updateMeter()
    {
        // Ask MeterManager to get new meter data
        meter->readMeter();
    }


    // ==================================================
    // Called when MeterManager sends new data
    // ==================================================

    void updateDisplay(MeterData data)
    {
        // Update voltage
        voltage->setText(
            QString("%1 V").arg(data.voltage, 0, 'f', 2));


        // Update current
        current->setText(
            QString("%1 A").arg(data.current, 0, 'f', 2));


        // Update power
        power->setText(
            QString("%1 kW").arg(data.power, 0, 'f', 2));


        // Update energy
        energy->setText(
            QString("%1 kWh").arg(data.energy, 0, 'f', 2));
    }


    // ==================================================
    // Start / Stop monitoring
    // ==================================================

    void toggleMonitoring()
    {
        // Check whether timer is currently running
        if (timer->isActive())
        {
            // Monitoring is currently ON
            // So stop it
            timer->stop();

            // Change button text
            monitorButton->setText("Monitoring: OFF");
            monitorButton->setProperty("monitoring", false);
        }
        else
        {
            // Monitoring is currently OFF
            // So start it
            timer->start(1000);

            // Change button text
            monitorButton->setText("Monitoring: ON");
            monitorButton->setProperty("monitoring", true);
        }

        monitorButton->style()->unpolish(monitorButton);
        monitorButton->style()->polish(monitorButton);
    }


private:

    // Title
    QLabel *title;

    // Parameter names
    QLabel *voltageName;
    QLabel *currentName;
    QLabel *powerName;
    QLabel *energyName;

    // Parameter values
    QLabel *voltage;
    QLabel *current;
    QLabel *power;
    QLabel *energy;

    // Monitoring button
    QPushButton *monitorButton;

    // Timer
    QTimer *timer;

    // Meter data manager
    MeterManager *meter;
};


// --------------------------------------------------
// Program starts here
// --------------------------------------------------
int main(int argc, char *argv[])
{
    // Create Qt application
    QApplication app(argc, argv);

    // Create MainWindow
    MainWindow window;

    // Set window title
    window.setWindowTitle("Smart Meter HMI");

    // Show window
    window.show();

    // Start Qt event loop
    return app.exec();
}

#include "main.moc"