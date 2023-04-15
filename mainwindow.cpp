#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1280, 720);
    setWindowTitle("PCz Rover Control Panel 2023");

    //emit drive->controlVirtualJoystick();

    //class' initialization ======================================================================================================================================================================================
    connection = new Connection(this);
    arm = new Arm(this);
    drive = new Drive(this);
    joystickWidget = new JoystickWidget(this);
    joystickWidget->setGeometry(326, 150, 250, 250);
    joystick = new DirectInputJoystick(this);

    connect(joystick, &DirectInputJoystick::connectedJoystickCountChanged, [this](int count) {
            ui->label_joysticks_count->setText(QString::number(count));
        });


    //timers initializations ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    uiTimer = new QTimer(this);
    connectionTimer = new QTimer(this);
    armTimer = new QTimer(this);
    driveTimer = new QTimer(this);
    frameStatusTimer = new QTimer(this);
    lastFrameSentTime = new QElapsedTimer();
    joystickPhysicalTimer = new QTimer(this);

    //lambdas ------------------------------------------------------------------------------------------------------------------------------------------------------------------
    auto connectionLambda =[=]()
    {
        connection->createFrame(drive->frame, arm->frame);
    };

    auto armVirtualLambda =[this]()
    {
        arm->containerX = static_cast<int> ((ui->horizontalSlider_x_arm->value() + 100.0) *65535.0 / 200.0);
        arm->containerY = static_cast<int> ((ui->horizontalSlider_y_arm->value() + 100.0) *65535.0 / 200.0);
        arm->containerZ = static_cast<int> ((ui->horizontalSlider_z_arm->value() + 100.0) *65535.0 / 200.0);
        arm->containerPower = ui->horizontalSlider_power_arm->value();

        arm->calculateSegmentsSpeeds(arm->containerX, arm->containerY, arm->containerZ, arm->containerPower, arm->activeButtonFunction, -1);
        //qDebug() <<arm->containerX<<arm->containerX/16383.0f;
        //arm->printButtonFunction(arm->activeButtonFunction);
        //arm->printButtonFunction(arm->buttonFunctionKeys[0].function);
        //arm->printButtonFunction(arm->buttonFunctionKeys[1].function);
    };
    connect(ui->pushButton_button_function_arm_1, &QPushButton::clicked, [this](){processButtonPressed(4,arm);});
    connect(ui->pushButton_button_function_arm_2, &QPushButton::clicked, [this](){processButtonPressed(5, arm);});
    connect(ui->pushButton_button_function_arm_3, &QPushButton::clicked, [this](){processButtonPressed(6, arm);});
    connect(ui->pushButton_button_function_arm_4, &QPushButton::clicked, [this](){processButtonPressed(7, arm);});
    connect(ui->pushButton_button_function_arm_5, &QPushButton::clicked, [this](){processButtonPressed(8, arm);});
    connect(ui->pushButton_button_function_arm_6, &QPushButton::clicked, [this](){processButtonPressed(9, arm);});


    auto armPhysicalLambda = [this]() {
        arm->calculateSegmentsSpeeds(arm->containerX, arm->containerY, arm->containerZ, arm->containerPower, arm->activeButtonFunction, arm->buttonPressed);
        updateUiArm(arm->containerX, arm->containerY, arm->containerZ, arm->containerPower);
        //qDebug() << arm->containerX << arm->containerY << arm->containerZ << arm->containerPower;
    };

    auto driveLambda =[this]()
    {
        drive->calculateWheelsSpeeds(drive->containerX, physicalJoystickMaxValue - drive->containerY, drive->containerPower);
        updateUiDrive(drive->containerX, drive->containerY, drive->containerPower);
    };

    auto armControlVirtualLambda =[=]()
    {
        disconnect(connArmTimer);
        disconnect(connArmPhysicalX);
        disconnect(connArmPhysicalY);
        disconnect(connArmPhysicalZ);
        disconnect(connArmPhysicalButton);
        connArmTimer = connect(armTimer, &QTimer::timeout, arm, armVirtualLambda);
    };

    auto armControlPhysical1Lambda =[=]()
    {
        disconnect(connArmTimer);
        disconnect(connArmPhysicalX);
        disconnect(connArmPhysicalY);
        disconnect(connArmPhysicalZ);
        disconnect(connArmPhysicalButton);
        connArmTimer = connect(armTimer, &QTimer::timeout, arm, armPhysicalLambda);
        connArmPhysicalX = connect(joystick, &DirectInputJoystick::joystick1AxisXChanged, [this](int value) { arm->containerX = value;});
        connArmPhysicalY = connect(joystick, &DirectInputJoystick::joystick1AxisYChanged, [this](int value) { arm->containerY = value;});
        connArmPhysicalZ = connect(joystick, &DirectInputJoystick::joystick1AxisZChanged, [this](int value) { arm->containerZ = value;});
        connArmPhysicalPower = connect(joystick, &DirectInputJoystick::joystick1SliderChanged, [this](int value) { arm->containerPower = ((value/physicalJoystickMaxValue)*100-100)*-1;});
        connArmPhysicalButton = connect(joystick, &DirectInputJoystick::joystick1ButtonStateChanged, [this](int button, bool pressed){arm->buttonPressed = button;
               processButtonPressed(button, arm);
        });
    };

    auto armControlPhysical2Lambda =[=]()
    {
        disconnect(connArmTimer);
        disconnect(connArmPhysicalX);
        disconnect(connArmPhysicalY);
        disconnect(connArmPhysicalZ);
        disconnect(connArmPhysicalButton);
        connArmTimer = connect(armTimer, &QTimer::timeout, arm, armPhysicalLambda);
        connArmPhysicalX = connect(joystick, &DirectInputJoystick::joystick2AxisXChanged, [this](int value) { arm->containerX = value;});
        connArmPhysicalY = connect(joystick, &DirectInputJoystick::joystick2AxisYChanged, [this](int value) { arm->containerY = value;});
        connArmPhysicalZ = connect(joystick, &DirectInputJoystick::joystick2AxisZChanged, [this](int value) { arm->containerZ = value;});
        connArmPhysicalPower = connect(joystick, &DirectInputJoystick::joystick2SliderChanged, [this](int value) { arm->containerPower = ((value/physicalJoystickMaxValue)*100-100)*-1;});
        connArmPhysicalButton = connect(joystick, &DirectInputJoystick::joystick2ButtonStateChanged, [this](int button, bool pressed){arm->buttonPressed = button;});
    };

    auto driveControlVirtualLambda =[=]()
    {
        disconnect(connDrivePhysicalX);
        disconnect(connDrivePhysicalY);
        disconnect(connDrivePhysicalPower);

        connDrivePhysicalX = connect(joystickWidget, &JoystickWidget::xChanged, [this](float value) { drive->containerX = (value+1.0)*32767; qDebug() <<value;  });
        connDrivePhysicalY = connect(joystickWidget, &JoystickWidget::yChanged, [this](float value) { drive->containerY = 65535 - (value+1.0)*32767;  });
        connDrivePhysicalPower = connect(ui->horizontalSlider_power_drive, &QSlider::valueChanged,[this](int value) { drive->containerPower = value;  });
    };

 auto driveControlPhysical1Lambda = [=]()
        {
            disconnect(connDrivePhysicalX);
            disconnect(connDrivePhysicalY);
            disconnect(connDrivePhysicalPower);
            connDrivePhysicalX = connect(joystick, &DirectInputJoystick::joystick1AxisXChanged, [this](int value) { drive->containerX = value;  });
            connDrivePhysicalY = connect(joystick, &DirectInputJoystick::joystick1AxisYChanged, [this](int value) { drive->containerY = value; });
            connDrivePhysicalPower = connect(joystick, &DirectInputJoystick::joystick1SliderChanged, [this](int value) { drive->containerPower = ((value/physicalJoystickMaxValue)*100-100)*-1; });
        };


    auto driveControlPhysical2Lambda =[=]()
    {
        disconnect(connDrivePhysicalX);
        disconnect(connDrivePhysicalY);
        disconnect(connDrivePhysicalPower);
        connDrivePhysicalX = connect(joystick, &DirectInputJoystick::joystick2AxisXChanged, [this](int value) { drive->containerX = value; });
        connDrivePhysicalY = connect(joystick, &DirectInputJoystick::joystick2AxisYChanged, [this](int value) { drive->containerY = value; });
        connDrivePhysicalPower = connect(joystick, &DirectInputJoystick::joystick2SliderChanged, [this](int value) { drive->containerPower = ((value/physicalJoystickMaxValue)*100-100)*-1; });
        updateUiDrive(drive->containerX, drive->containerY, drive->containerPower);
    };


    auto handleButtonFunction = [this](int buttonNumber)
    {
        for (int i = 0; i < 6; i++)
        {
            if (i == buttonNumber - 1)
            {
                arm->buttonFunctionKeys[i].isActive = true;
            }
            else
            {
                arm->buttonFunctionKeys[i].isActive = false;
            }
        }
    };

    // Define a lambda function to set the ButtonFunctionKey function property based on the selected QComboBox index
    auto setButtonFunction = [](QComboBox* comboBox, Arm::ButtonFunctionKey& buttonFunctionKey) {
        int index = comboBox->currentIndex();
        switch (index)
        {
            case 0:
            buttonFunctionKey.function = Arm::ButtonFunction::none;
                break;
            case 1:
            buttonFunctionKey.function = Arm::ButtonFunction::first;
                break;
            case 2:
            buttonFunctionKey.function = Arm::ButtonFunction::second;
                break;
            case 3:
            buttonFunctionKey.function = Arm::ButtonFunction::third;
                break;
            case 4:
            buttonFunctionKey.function = Arm::ButtonFunction::jaws;
                break;
            case 5:
            buttonFunctionKey.function = Arm::ButtonFunction::all;
                break;
            case 6:
            buttonFunctionKey.function = Arm::ButtonFunction::inverseKinematics;
                break;
            default:
            buttonFunctionKey.function = Arm::ButtonFunction::none;
                break;
        }
    };



//BUSINESS ======================================================================================================================================================================================
connect(connectionTimer, &QTimer::timeout, connection, connectionLambda);
connDrivePhysicalX = connect(joystickWidget, &JoystickWidget::xChanged, [this](float value) { drive->containerX = (value+1.0)*32767; qDebug() <<value;  });
connDrivePhysicalY = connect(joystickWidget, &JoystickWidget::yChanged, [this](float value) { drive->containerY = 65535 - (value+1.0)*32767;  });
connDrivePhysicalPower = connect(ui->horizontalSlider_power_drive, &QSlider::valueChanged,[this](int value) { drive->containerPower = value;  });


connect(driveTimer, &QTimer::timeout, drive, driveLambda);

//Drive with widgets------------------------------------------------------------------------------------------------------------------------------------------------------------------
connect(drive, &Drive::controlVirtualJoystick, drive, driveControlVirtualLambda);

//Drive with joystick------------------------------------------------------------------------------------------------------------------------------------------------------------------
connect(drive, &Drive::controlPhysicalJoystick1, drive, driveControlPhysical1Lambda);
connect(drive, &Drive::controlPhysicalJoystick2, drive, driveControlPhysical2Lambda);

//Arm with widgets------------------------------------------------------------------------------------------------------------------------------------------------------------------
connArmTimer = connect(armTimer, &QTimer::timeout, arm, armVirtualLambda);
connect(arm, &Arm::controlVirtualSliders, arm, armControlVirtualLambda);
// Connect the button click events to the lambda function
connect(ui->pushButton_button_function_arm_1, &QPushButton::clicked, [this, handleButtonFunction]()
{
    handleButtonFunction(1);
    });
connect(ui->pushButton_button_function_arm_2, &QPushButton::clicked, [this, handleButtonFunction]()
{
    handleButtonFunction(2);
    });
connect(ui->pushButton_button_function_arm_3, &QPushButton::clicked, [this, handleButtonFunction]()
{
    handleButtonFunction(3);
    });
connect(ui->pushButton_button_function_arm_4, &QPushButton::clicked, [this, handleButtonFunction]()
{
    handleButtonFunction(4);
    });
connect(ui->pushButton_button_function_arm_5, &QPushButton::clicked, [this, handleButtonFunction]()
{
    handleButtonFunction(5);
    });
connect(ui->pushButton_button_function_arm_6, &QPushButton::clicked, [this, handleButtonFunction]()
{
    handleButtonFunction(6);
    });
//Arm with joystick------------------------------------------------------------------------------------------------------------------------------------------------------------------
connect(arm, &Arm::controlPhysicalJoystick1, arm, armControlPhysical1Lambda);
connect(arm, &Arm::controlPhysicalJoystick2, arm, armControlPhysical2Lambda);

connect(frameStatusTimer, &QTimer::timeout, this, &MainWindow::updateFrameStatusLabel);
connect(connection, &Connection::frameSent, this,
[=]()
    {
        ui->label_frames_sent_count->setText(QString::number(connection->framesSent));
        lastFrameSentTime->restart();

        if (!frameStatusTimer->isActive())
        {
            frameStatusTimer->start(100);
        } });
connect(uiTimer, &QTimer::timeout, this, [=]()
{
    //wheel speeds
    ui->progressBar_wheel_left_back->setValue(drive->getLeftSpeed());
    ui->progressBar_wheel_left_middle->setValue(drive->getLeftSpeed());
    ui->progressBar_wheel_left_front->setValue(drive->getLeftSpeed());
    ui->progressBar_wheel_right_back->setValue(drive->getRightSpeed());
    ui->progressBar_wheel_right_middle->setValue(drive->getRightSpeed());
    ui->progressBar_wheel_right_front->setValue(drive->getRightSpeed());
    setButtonFunction(ui->comboBox_arm_1, arm->buttonFunctionKeys[0]);
    setButtonFunction(ui->comboBox_arm_2, arm->buttonFunctionKeys[1]);
    setButtonFunction(ui->comboBox_arm_3, arm->buttonFunctionKeys[2]);
    setButtonFunction(ui->comboBox_arm_4, arm->buttonFunctionKeys[3]);
    setButtonFunction(ui->comboBox_arm_5, arm->buttonFunctionKeys[4]);
    setButtonFunction(ui->comboBox_arm_6, arm->buttonFunctionKeys[5]);


    });
//UI INITIALIZATION======================================================================================================================================================================================
ui->label_frames_sent_count->setStyleSheet("background-color: rgb(160,20,40);");
connect(arm, &Arm::buttonFunctionChanged, this, &MainWindow::updateButtonColors);
//UI Drive------------------------------------------------------------------------------------------------------------------------------------------------------------------
connect(ui->horizontalSlider_power_drive, &QSlider::valueChanged, this, [=]()
{
    ui->spinBox_power_drive->setValue(ui->horizontalSlider_power_drive->value());
    });
connect(ui->spinBox_power_drive, &QSpinBox::valueChanged, this, [=]()
{
    ui->horizontalSlider_power_drive->setValue(ui->spinBox_power_drive->value());
    });
connect(ui->horizontalSlider_x_drive, &QSlider::valueChanged, this, [=]()
{
    ui->label_x_drive->setText(QString::number(ui->horizontalSlider_x_drive->value()));
    });
connect(ui->horizontalSlider_y_drive, &QSlider::valueChanged, this, [=]()
{
    ui->label_y_drive->setText(QString::number(ui->horizontalSlider_y_drive->value()));
    });

//UI arm------------------------------------------------------------------------------------------------------------------------------------------------------------------
connect(ui->horizontalSlider_power_arm, &QSlider::valueChanged, this, [=]()
{
    ui->spinBox_power_arm->setValue(ui->horizontalSlider_power_arm->value());
    });
connect(ui->spinBox_power_arm, &QSpinBox::valueChanged, this, [=]()
{
    ui->horizontalSlider_power_arm->setValue(ui->spinBox_power_arm->value());
    });
connect(ui->horizontalSlider_x_arm, &QSlider::valueChanged, this, [=]()
{
    ui->label_x_arm->setText(QString::number(ui->horizontalSlider_x_arm->value()));
    });
connect(ui->horizontalSlider_y_arm, &QSlider::valueChanged, this, [=]()
{
    ui->label_y_arm->setText(QString::number(ui->horizontalSlider_y_arm->value()));
    });
connect(ui->horizontalSlider_z_arm, &QSlider::valueChanged, this, [=]()
{
    ui->label_z_arm->setText(QString::number(ui->horizontalSlider_z_arm->value()));
    });

//timers start------------------------------------------------------------------------------------------------------------------------------------------------------------------
uiTimer->start(1000 / 60);
driveTimer->start(1000 / 60);
armTimer->start(1000 / 60);
connectionTimer->start(33);
joystickPhysicalTimer->start(500);
lastFrameSentTime->start();

//UI updates------------------------------------------------------------------------------------------------------------------------------------------------------------------
connect(connection, &Connection::frameSent, this,
[=]()
    {
        ui->label_frames_sent_count->setText(QString::number(connection->framesSent));
    });
}



MainWindow::~MainWindow()
{
    delete connectionTimer;
    delete driveTimer;

    delete connection;
    delete drive;
    delete joystickWidget;
    delete ui;
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

void MainWindow::updateUiArm(int containerX, int containerY, int containerZ, int containerPower) {
    ui->horizontalSlider_x_arm->setValue(containerX / 327.67 - 100);
    ui->horizontalSlider_y_arm->setValue(containerY / 327.67 - 100);
    ui->horizontalSlider_z_arm->setValue(containerZ / 327.67 - 100);
    ui->horizontalSlider_power_arm->setValue(containerPower);
}

void MainWindow::updateUiDrive(int containerX, int containerY, int containerPower) {
    ui->horizontalSlider_x_drive->setValue(containerX / 327.67 - 100);
    ui->horizontalSlider_y_drive->setValue(containerY / 327.67 - 100);
    ui->horizontalSlider_power_drive->setValue(containerPower);
}

void MainWindow::processButtonPressed(int buttonPressedNow, Arm* arm) {
    arm->buttonPressed = buttonPressedNow;

    for (int buttonFunctionKeyNumber = 0; buttonFunctionKeyNumber < 6; buttonFunctionKeyNumber++) {
        if (buttonPressedNow >= 4 && buttonPressedNow <= 9) {
            if (buttonPressedNow - 4 == buttonFunctionKeyNumber) {
                arm->buttonFunctionKeys[buttonFunctionKeyNumber].isActive = true;

            } else {
                arm->buttonFunctionKeys[buttonFunctionKeyNumber].isActive = false;
            }
        }
    }
    emit arm->buttonFunctionChanged();
}

// Function to update the button colors
void MainWindow::updateButtonColors()
{
    for (int i = 0; i < 6; ++i)
    {
        // Find the button by its objectName
        QString buttonName = QString("pushButton_button_function_arm_%1").arg(i + 1);
        QPushButton *button = centralWidget()->findChild<QPushButton *>(buttonName);

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



