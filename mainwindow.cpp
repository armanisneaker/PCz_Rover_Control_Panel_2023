#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    setWindowTitle("PCz Rover Control Panel 2023");

    initializeClasses();
    initializeTimers();
    initializeBusiness();
    initializeUi();

    startTimers();

    //
    //podpięcie mapki do robienia cudów
    mapmodule = new MapModule(this);
    ui->graphicsView->setScene(mapmodule);

    ui->graphicsView->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->graphicsView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    scale.viewtransform = ui->graphicsView->transform();
    scale.scale = scale_enum::m1;
}

MainWindow::~MainWindow()
{
    deleteClasses();
    deleteTimers();
}


void MainWindow::updateFrameStatusLabel()
{
    timeSinceLastFrameSent = lastFrameSentTime->elapsed();
    QString color;

    if (timeSinceLastFrameSent < 333)
    {
        color = "background-color: rgb(66,158,88);";
    }
    else if (timeSinceLastFrameSent < 1000)
    {
        color = "background-color: rgb(255,181,10);";
    }
    else
    {
        color = "background-color: rgb(160,20,40);";
    }

    ui->label_frames_sent_count->setStyleSheet(color);
}

void MainWindow::on_comboBox_drive_control_currentTextChanged(const QString &arg1)
{
    if (arg1 == "Virtual Joystick") emit drive->controlVirtualJoystick();
    else if (arg1 == "Physical Joystick 1") emit drive->controlPhysicalJoystick1();
    else if (arg1 == "Physical Joystick 2") emit drive->controlPhysicalJoystick2();
}

void MainWindow::on_comboBox_arm_control_currentTextChanged(const QString &arg1)
{
    if (arg1 == "Virtual Sliders") emit arm->controlVirtualSliders();
    else if (arg1 == "Physical Joystick 1") emit arm->controlPhysicalJoystick1();
    else if (arg1 == "Physical Joystick 2") emit arm->controlPhysicalJoystick2();
}

void MainWindow::on_horizontalSlider_x_arm_sliderReleased()
{
    ui->horizontalSlider_x_arm->setValue(0);
}

void MainWindow::on_horizontalSlider_y_arm_sliderReleased()
{
    ui->horizontalSlider_y_arm->setValue(0);
}

void MainWindow::on_horizontalSlider_z_arm_sliderReleased()
{
    ui->horizontalSlider_z_arm->setValue(0);
}

void MainWindow::on_horizontalSlider_x_drive_sliderReleased()
{
    ui->horizontalSlider_x_drive->setValue(0);
}

void MainWindow::on_horizontalSlider_y_drive_sliderReleased()
{
    ui->horizontalSlider_y_drive->setValue(0);
}

void MainWindow::updateUiArm(int containerX, int containerY, int containerZ, int containerPower, bool xReleased, bool yReleased, bool zReleased)
{
    if (!xReleased) {
        ui->horizontalSlider_x_arm->setValue(containerX / 327.67 - 100);
    }
    if (!yReleased) {
        ui->horizontalSlider_y_arm->setValue(containerY / 327.67 - 100);
    }
    if (!zReleased) {
        ui->horizontalSlider_z_arm->setValue(containerZ / 327.67 - 100);
    }
    ui->horizontalSlider_power_arm->setValue(containerPower);
}



void MainWindow::updateUiDrive(int containerX, int containerY, int containerPower)
{
    ui->horizontalSlider_x_drive->setValue(containerX / 327.67 - 100);
    ui->horizontalSlider_y_drive->setValue(containerY / 327.67 - 100);
    ui->horizontalSlider_power_drive->setValue(containerPower);
}

// Function to update the button colors
void MainWindow::updateButtonColors()
{
    for (int i = 0; i < 6; ++i)
    {
        // Find the button by its objectName
        QString buttonName = QString("pushButton_button_function_arm_%1").arg(i + 1);
        QPushButton *button = centralWidget()->findChild<QPushButton*> (buttonName);

        // Check if the button was found
        if (button)
        {
            // Set the button color to green if isActive is true, otherwise set it to red
            QColor color = arm->buttonFunctionKeys[i].isActive ? QColor(Qt::darkGreen) : QColor(Qt::red);

            // Update the button style with the new color
            button->setStyleSheet(QString("background-color: %1").arg(color.name()));
        }
    }
}

void MainWindow::initializeClasses()
{
    connection = new Connection(this);
    arm = new Arm(this);
    drive = new Drive(this);
    joystickWidget = new JoystickWidget(this);
    joystickWidget->setGeometry(27, 485, 250, 250);
    joystick = new DirectInputJoystick(this);

    printToUi("Classes initialized");
}

void MainWindow::initializeTimers()
{
    uiTimer = new QTimer(this);
    connectionTimer = new QTimer(this);
    armTimer = new QTimer(this);
    driveTimer = new QTimer(this);
    frameStatusTimer = new QTimer(this);
    lastFrameSentTime = new QElapsedTimer();
    joystickPhysicalTimer = new QTimer(this);

    printToUi("Timers initialized");
}

void MainWindow::startTimers()
{
    uiTimer->start(1000 / 60);
    driveTimer->start(1000 / 60);
    armTimer->start(1000 / 60);
    connectionTimer->start(1000 / 60);
    joystickPhysicalTimer->start(500);
    lastFrameSentTime->start();

    printToUi("Timers started");
}

void MainWindow::initializeBusiness()
{
    setupBusinessConnections();

    emit arm->controlVirtualSliders();

    ui->label_frames_sent_count->setStyleSheet("background-color: rgb(160,20,40);");
    updateButtonColors();
    ui->comboBox_arm_control->setCurrentIndex(1);
    ui->comboBox_drive_control->setCurrentIndex(2);

    printToUi("Business initialized");
}

void MainWindow::initializeUi()
{
    setupUiConnections();
    printToUi("UI initialized");
}

void MainWindow::setupBusinessConnections()
{
    connect(connectionTimer, &QTimer::timeout, this, &MainWindow::connectionSlot);
    connArmTimer = connect(armTimer, &QTimer::timeout, this, &MainWindow::armSlot);
    connect(arm, &Arm::controlVirtualSliders, this, &MainWindow::armControlVirtualSlot);
    connect(arm, &Arm::controlPhysicalJoystick1, this, &MainWindow::armControlPhysical1Slot);
    connect(arm, &Arm::controlPhysicalJoystick2, this, &MainWindow::armControlPhysical2Slot);
    connect(arm, &Arm::buttonFunctionChanged, this, [=](int index) {
        switch (index) {
            case 0:
                setButtonFunction(ui->comboBox_arm_1, arm->buttonFunctionKeys[index]);
                break;
            case 1:
                setButtonFunction(ui->comboBox_arm_2, arm->buttonFunctionKeys[index]);
                break;
            case 2:
                setButtonFunction(ui->comboBox_arm_3, arm->buttonFunctionKeys[index]);
                break;
            case 3:
                setButtonFunction(ui->comboBox_arm_4, arm->buttonFunctionKeys[index]);
                break;
            case 4:
                setButtonFunction(ui->comboBox_arm_5, arm->buttonFunctionKeys[index]);
                break;
            case 5:
                setButtonFunction(ui->comboBox_arm_6, arm->buttonFunctionKeys[index]);
                break;
        }
    });
    connect(arm, &Arm::buttonFunctionChanged, this, [this]()
    {
        for (int i = 0; i < 6; i++)
        {
            if (arm->buttonFunctionKeys[i].isActive == true) arm->activeButtonFunction = arm->buttonFunctionKeys[i].function;
        } });
    connect(ui->comboBox_arm_1, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int newIndex) {
        arm->onButtonFunctionIndexChanged(0, newIndex);
    });
    connect(ui->comboBox_arm_2, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int newIndex) {
        arm->onButtonFunctionIndexChanged(1, newIndex);
    });
    connect(ui->comboBox_arm_3, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int newIndex) {
        arm->onButtonFunctionIndexChanged(2, newIndex);
    });
    connect(ui->comboBox_arm_4, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int newIndex) {
        arm->onButtonFunctionIndexChanged(3, newIndex);
    });
    connect(ui->comboBox_arm_5, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int newIndex) {
        arm->onButtonFunctionIndexChanged(4, newIndex);
    });
    connect(ui->comboBox_arm_6, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int newIndex) {
        arm->onButtonFunctionIndexChanged(5, newIndex);
    });
    connectArmButtons();

    connDriveTimer = connect(driveTimer, &QTimer::timeout, this, &MainWindow::driveSlot);
    connect(drive, &Drive::controlVirtualJoystick, this, &MainWindow::driveControlVirtualSlot);
    connect(drive, &Drive::controlPhysicalJoystick1, this, &MainWindow::driveControlPhysical1Slot);
    connect(drive, &Drive::controlPhysicalJoystick2, this, &MainWindow::driveControlPhysical2Slot);

    // JoystickWidget connections
    connDrivePhysicalX = connect(joystickWidget, &JoystickWidget::xChanged, this, [this](float value)
    {
        drive->containerX = (value + 1.0) *32767;
    });
    connDrivePhysicalY = connect(joystickWidget, &JoystickWidget::yChanged, this, [this](float value)
    {
        drive->containerY = 65535 - (value + 1.0) *32767;
    });

    // Drive power slider
    connDrivePhysicalPower = connect(ui->horizontalSlider_power_drive, &QSlider::valueChanged, this, [this](int value)
    {
        drive->containerPower = value;
    });

    connect(ui->lineEdit_IP_adress, &QLineEdit::editingFinished, this,[this]{
        connection->setHostAddress(ui->lineEdit_IP_adress->text());
    } );
    connect(ui->lineEdit_port, &QLineEdit::editingFinished, this, [this]{
        bool conversionSuccessful;
        int integerValue = ui->lineEdit_port->text().toInt(&conversionSuccessful);
        connection->hostPortChanged(ui->lineEdit_port->text().toInt(&conversionSuccessful));
        if (conversionSuccessful)
        {
            if (integerValue >= std::numeric_limits<qint16>::min() && integerValue <= std::numeric_limits<qint16>::max())
            {
                qint16 convertedValue = static_cast<qint16>(integerValue);
            }
            else
            {
                 printToUi("The PORT input number is out of the qint16 range.");
            }
        }
        else
        {
            qDebug() << "Failed to convert QString to qint16 for PORT.";
        }
    });
}

void MainWindow::setupUiConnections()
{
    // Update joysticks count label
    connect(joystick, &DirectInputJoystick::connectedJoystickCountChanged, this, [this](int count)
    {
        ui->label_joysticks_count->setText(QString::number(count));
    });
    // Frame status timer connection
    connect(frameStatusTimer, &QTimer::timeout, this, &MainWindow::updateFrameStatusLabel);
    // Connection frame sent handling
    connect(connection, &Connection::frameSent, this, [=]()
    {
        ui->label_frames_sent_count->setText(QString::number(connection->framesSent));
        lastFrameSentTime->restart();

        if (!frameStatusTimer->isActive())
        {
            frameStatusTimer->start(100);
        } });

    // UI timer connections
    // Assuming that drive is a pointer to a Drive object and is properly initialized
    connect(drive, &Drive::leftSpeedChanged, this, [=](int16_t leftSpeed) {
        ui->progressBar_wheel_left_back->setValue(leftSpeed);
        ui->progressBar_wheel_left_middle->setValue(leftSpeed);
        ui->progressBar_wheel_left_front->setValue(leftSpeed);
    });

    connect(drive, &Drive::rightSpeedChanged, this, [=](int16_t rightSpeed) {
        ui->progressBar_wheel_right_back->setValue(rightSpeed);
        ui->progressBar_wheel_right_middle->setValue(rightSpeed);
        ui->progressBar_wheel_right_front->setValue(rightSpeed);
    });
    connect(arm, &Arm::buttonFunctionChanged, this, &MainWindow::updateButtonColors);
    connect(ui->spinBox_power_drive, &QSpinBox::valueChanged, this, [=](int value)
    {
        ui->horizontalSlider_power_drive->setValue(value);
    });
    connect(ui->horizontalSlider_x_drive, &QSlider::valueChanged, this, [=](int value)
    {
        ui->label_x_drive->setText(QString::number(value));
    });
    connect(ui->horizontalSlider_y_drive, &QSlider::valueChanged, this, [=](int value)
    {
        ui->label_y_drive->setText(QString::number(value));
    });
    connect(ui->horizontalSlider_power_drive, &QSlider::valueChanged, this, [=](int value)
    {
        ui->spinBox_power_drive->setValue(value);
    });

    connect(ui->horizontalSlider_power_arm, &QSlider::valueChanged, this, [=](int value)
    {
        ui->spinBox_power_arm->setValue(value);
    });
    connect(ui->spinBox_power_arm, &QSpinBox::valueChanged, this, [=](int value)
    {
        ui->horizontalSlider_power_arm->setValue(value);
    });
    connect(ui->horizontalSlider_x_arm, &QSlider::valueChanged, this, [=](int value)
    {
        ui->label_x_arm->setText(QString::number(value));
    });
    connect(ui->horizontalSlider_y_arm, &QSlider::valueChanged, this, [=](int value)
    {
        ui->label_y_arm->setText(QString::number(value));
    });
    connect(ui->horizontalSlider_z_arm, &QSlider::valueChanged, this, [=](int value)
    {
        ui->label_z_arm->setText(QString::number(value));
    });
    connect(arm, &Arm::motorBaseChanged, ui->progressBar_motor_base, &QProgressBar::setValue);
    connect(arm, &Arm::motorFirstChanged, ui->progressBar_motor_first, &QProgressBar::setValue);
    connect(arm, &Arm::motorSecondChanged, ui->progressBar_motor_second, &QProgressBar::setValue);
    connect(arm, &Arm::motorThirdChanged, ui->progressBar_motor_third, &QProgressBar::setValue);
    connect(arm, &Arm::motorJawsChanged, ui->progressBar_motor_jaws, &QProgressBar::setValue);
    connect(arm, &Arm::motorJawsClenchChanged, ui->progressBar_motor_jaws_clench, &QProgressBar::setValue);
    // Frame sent count update
    connect(connection, &Connection::frameSent, this, [=]()
    {
        ui->label_frames_sent_count->setText(QString::number(connection->framesSent));
    });

    connect(drive, &Drive::roverSpeedChanged, this, [this](int value){
        ui->lcdNumber_rover_speed->display(value);
        ui->dial_rover_speed->setValue(value);
        //printToUi(QString::number(value));
    } );
    connect(connection, &Connection::frameFailedToBeSent, this, [this](int value){
        printToUi("Frame failed to be sent: bytes:" + QString::number(value));});
}

void MainWindow::connectionSlot()
{
    connection->createFrame(drive->frame, arm->frame);
}

void MainWindow::armSlot()
{
    arm->calculateSegmentsSpeeds(arm->containerX, arm->containerY, arm->containerZ, arm->containerPower, arm->activeButtonFunction, arm->buttonPressed);
    updateUiArm(arm->containerX, arm->containerY, arm->containerZ, arm->containerPower, xReleased, yReleased, zReleased);
}

void MainWindow::armControlVirtualSlot()
{
    disconnect(connArmPhysicalX);
    disconnect(connArmPhysicalY);
    disconnect(connArmPhysicalZ);
    disconnect(connArmPhysicalPower);
    disconnect(connArmPhysicalButton);
    disconnect(connArmButtonRelease);
    disconnect(connArmReleasedX);
    disconnect(connArmReleasedY);
    disconnect(connArmReleasedZ);

xReleased = false;
        yReleased = false;
        zReleased = false;

        connArmPhysicalX = connect(ui->horizontalSlider_x_arm, &QSlider::sliderMoved, this, [this](int value)
        {
            xReleased = false;
            arm->containerX = (value + 100) * 327.67;
        });
        connArmPhysicalY = connect(ui->horizontalSlider_y_arm, &QSlider::sliderMoved, this, [this](int value)
        {
            yReleased = false;
            arm->containerY = (value + 100) * 327.67;
        });
        connArmPhysicalZ = connect(ui->horizontalSlider_z_arm, &QSlider::sliderMoved, this, [this](int value)
        {
            zReleased = false;
            arm->containerZ = (value + 100) * 327.67;
        });
        connArmPhysicalPower = connect(ui->horizontalSlider_power_arm, &QSlider::sliderMoved, this, [this](int value)
        {
            arm->containerPower = value;
        });
        connArmReleasedX = connect(ui->horizontalSlider_x_arm, &QSlider::sliderReleased, this, [this]()
        {
            xReleased = true;
            arm->containerX = (100) * 327.67;
        });
        connArmReleasedY = connect(ui->horizontalSlider_y_arm, &QSlider::sliderReleased, this, [this]()
        {
            yReleased = true;
            arm->containerY = (100) * 327.67;
        });
        connArmReleasedZ = connect(ui->horizontalSlider_z_arm, &QSlider::sliderReleased, this, [this]()
        {
            zReleased = true;
            arm->containerZ = (100) * 327.67;
        });
}

void MainWindow::armControlPhysical1Slot()
{
    // Disconnect existing connections
    disconnect(connArmPhysicalX);
    disconnect(connArmPhysicalY);
    disconnect(connArmPhysicalZ);
    disconnect(connArmPhysicalPower);
    disconnect(connArmPhysicalButton);
    disconnect(connArmButtonRelease);
    disconnect(connArmReleasedX);
    disconnect(connArmReleasedY);
    disconnect(connArmReleasedZ);

    // Connect joystick signals to their respective slots
    connArmPhysicalX = connect(joystick, &DirectInputJoystick::joystick1AxisXChanged, this, [this](int value)
    {
        arm->containerX = value;
    });
    connArmPhysicalY = connect(joystick, &DirectInputJoystick::joystick1AxisYChanged, this, [this](int value)
    {
        arm->containerY = 2*32768-value;
    });
    connArmPhysicalZ = connect(joystick, &DirectInputJoystick::joystick1AxisZChanged, this, [this](int value)
    {
        arm->containerZ = value;
    });
    connArmPhysicalPower = connect(joystick, &DirectInputJoystick::joystick1SliderChanged, this, [this](int value)
    {
        arm->containerPower = ((value / physicalJoystickMaxValue) *100 - 100) *-1;
    });
    connArmPhysicalButton = connect(joystick, &DirectInputJoystick::joystick1ButtonStateChanged, this, [this](int button, bool pressed)
    {
        arm->buttonPressed = button;
        arm->processButtonPressed(button);
    });
    connArmButtonRelease = connect(joystick, &DirectInputJoystick::joystick1ButtonReleased, this, [this]{
        arm->buttonPressed = 8;
        arm->processButtonPressed(8);
    });
}

void MainWindow::armControlPhysical2Slot()
{
    // Disconnect existing connections
    disconnect(connArmPhysicalX);
    disconnect(connArmPhysicalY);
    disconnect(connArmPhysicalZ);
    disconnect(connArmPhysicalPower);
    disconnect(connArmPhysicalButton);
    disconnect(connArmButtonRelease);
    disconnect(connArmReleasedX);
    disconnect(connArmReleasedY);
    disconnect(connArmReleasedZ);

    // Connect joystick signals to their respective slots
    connArmPhysicalX = connect(joystick, &DirectInputJoystick::joystick2AxisXChanged, this, [this](int value)
    {
        arm->containerX = value;
    });
    connArmPhysicalY = connect(joystick, &DirectInputJoystick::joystick2AxisYChanged, this, [this](int value)
    {
        arm->containerY = value;
    });
    connArmPhysicalZ = connect(joystick, &DirectInputJoystick::joystick2AxisZChanged, this, [this](int value)
    {
        arm->containerZ = value;
    });
    connArmPhysicalPower = connect(joystick, &DirectInputJoystick::joystick2SliderChanged, this, [this](int value)
    {
        arm->containerPower = ((value / physicalJoystickMaxValue) *100 - 100) *-1;
    });
    connArmPhysicalButton = connect(joystick, &DirectInputJoystick::joystick2ButtonStateChanged, this, [this](int button, bool pressed)
    {
        arm->buttonPressed = button;
    });
    connArmButtonRelease = connect(joystick, &DirectInputJoystick::joystick2ButtonReleased, this, [this]{
        arm->buttonPressed = 8;
        arm->processButtonPressed(8);
    });
}

void MainWindow::connectArmButtons()
{
    for (int i = 1; i <= 6; ++i)
    {
        QPushButton *button = this->findChild<QPushButton*> (QString("pushButton_button_function_arm_%1").arg(i));
        connect(button, &QPushButton::clicked, this, [this, i]()
        {
            arm->processButtonPressed(i + 3);
        });
    }
}

void MainWindow::setButtonFunction(QComboBox *comboBox, Arm::ButtonFunctionKey &buttonFunctionKey)
{
    int index = comboBox->currentIndex();
    switch (index)
    {
        case 0:
            buttonFunctionKey.function = Arm::ButtonFunction::None;
            break;
        case 1:
            buttonFunctionKey.function = Arm::ButtonFunction::First;
            break;
        case 2:
            buttonFunctionKey.function = Arm::ButtonFunction::Second;
            break;
        case 3:
            buttonFunctionKey.function = Arm::ButtonFunction::Third;
            break;
        case 4:
            buttonFunctionKey.function = Arm::ButtonFunction::Jaws;
            break;
        case 5:
            buttonFunctionKey.function = Arm::ButtonFunction::All;
            break;
        case 6:
            buttonFunctionKey.function = Arm::ButtonFunction::InverseKinematics;
            break;
        default:
            buttonFunctionKey.function = Arm::ButtonFunction::None;
            break;
    }
}

void MainWindow::driveSlot()
{
    drive->calculateWheelsSpeeds(drive->containerX, physicalJoystickMaxValue - drive->containerY, drive->containerPower);
    updateUiDrive(drive->containerX, drive->containerY, drive->containerPower);
}

void MainWindow::driveControlVirtualSlot()
{
    disconnect(connDrivePhysicalX);
    disconnect(connDrivePhysicalY);
    disconnect(connDrivePhysicalPower);

    connDrivePhysicalX = connect(joystickWidget, &JoystickWidget::xChanged, this, [this](float value)
    {
        drive->containerX = (value + 1.0) * 32767;
    });
    connDrivePhysicalY = connect(joystickWidget, &JoystickWidget::yChanged, this, [this](float value)
    {
        drive->containerY = 65535 - (value + 1.0) * 32767;
    });
    connDrivePhysicalPower = connect(ui->horizontalSlider_power_drive, &QSlider::valueChanged, this, [this](int value)
    {
        drive->containerPower = value;
    });
}

void MainWindow::driveControlPhysical1Slot()
{
    disconnect(connDrivePhysicalX);
    disconnect(connDrivePhysicalY);
    disconnect(connDrivePhysicalPower);

    connDrivePhysicalX = connect(joystick, &DirectInputJoystick::joystick1AxisXChanged, this, [this](int value)
    {
        drive->containerX = value;
    });
    connDrivePhysicalY = connect(joystick, &DirectInputJoystick::joystick1AxisYChanged, this, [this](int value)
    {
        drive->containerY = value;
    });
    connDrivePhysicalPower = connect(joystick, &DirectInputJoystick::joystick1SliderChanged, this, [this](int value)
    {
        drive->containerPower = ((value / physicalJoystickMaxValue) * 100 - 100) * -1;
    });
}

void MainWindow::driveControlPhysical2Slot()
{
    disconnect(connDrivePhysicalX);
    disconnect(connDrivePhysicalY);
    disconnect(connDrivePhysicalPower);

    connDrivePhysicalX = connect(joystick, &DirectInputJoystick::joystick2AxisXChanged, this, [this](int value)
    {
        drive->containerX = value;
    });
    connDrivePhysicalY = connect(joystick, &DirectInputJoystick::joystick2AxisYChanged, this, [this](int value)
    {
        drive->containerY = value;
    });
    connDrivePhysicalPower = connect(joystick, &DirectInputJoystick::joystick2SliderChanged, this, [this](int value)
    {
        drive->containerPower = ((value / physicalJoystickMaxValue) * 100 - 100) * -1;
    });
    updateUiDrive(drive->containerX, drive->containerY, drive->containerPower);
}

void MainWindow::printToUi(QString text)
{
    ui->textBrowser_printer->append(QDateTime::currentDateTime().toString("HH:mm:ss") + " " + text);
}

void MainWindow::deleteClasses()
{
    delete connection;
    delete arm;
    delete drive;
    delete joystickWidget;
    delete joystick;
}

void MainWindow::deleteTimers()
{
    delete uiTimer;
    delete connectionTimer;
    delete armTimer;
    delete driveTimer;
    delete frameStatusTimer;
    delete lastFrameSentTime;
    delete joystickPhysicalTimer;
}


void MainWindow::on_pushButtonUnZoom_clicked()
{
    //zaczyna od zerowej transformacji

    switch(scale.scale)
    {
        case scale_enum::m1:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m10;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("10m");
            ui->graphicsView->scale(0.1,0.1);
            break;
        }
        case scale_enum::m10:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m50;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("50m");
            ui->graphicsView->scale(0.02,0.02);
            break;
        }
        case scale_enum::m50:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m100;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("50m");
            ui->graphicsView->scale(0.01,0.01);
            break;
        }
        case scale_enum::m100:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m500;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("50m");
            ui->graphicsView->scale(0.005,0.005);
            break;
        }
        case scale_enum::m500:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m1000;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("100m");
            ui->graphicsView->scale(0.001,0.001);
            break;

        }

        case scale_enum::m1000:
        {
            break;
        }

        default:
        {
            break;
        }
    }
}


void MainWindow::on_pushButtonZoom_clicked()
{
    //zaczyna od zerowej transformacji


    switch(scale.scale)
    {

        case scale_enum::m1:
        {
            ui->label_scale->setText("1m");
            break;
        }
        case scale_enum::m10:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m1;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("1m");
            ui->graphicsView->scale(1,1);
            break;
        }
        case scale_enum::m50:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m10;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("10m");
            ui->graphicsView->scale(0.1,0.1);
            break;
        }
        case scale_enum::m100:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m50;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("50m");
            ui->graphicsView->scale(0.02,0.02);
            break;
        }
        case scale_enum::m500:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m100;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("50m");
            ui->graphicsView->scale(0.01,0.01);
            break;
        }
        case scale_enum::m1000:
        {
            ui->graphicsView->resetTransform();
            scale.scale = scale_enum::m500;
            mapmodule->scale(scale.scale);
            ui->label_scale->setText("50m");
            ui->graphicsView->scale(0.005,0.005);
            break;
        }

        default:
        {
            break;
        }
    }
}


void MainWindow::on_pushButtonSend_clicked()
{
    mapmodule->UpdatePosition(ui->lineEditPosXRover->text().toDouble(),ui->lineEditPosYRover->text().toDouble());
    ui->graphicsView->centerOn(mapmodule->size_of_map/2,mapmodule->size_of_map/2);
}


void MainWindow::on_pushButtonRotate_clicked()
{
    mapmodule->rover.rotate(ui->lineEditRotate->text().toInt());
}
