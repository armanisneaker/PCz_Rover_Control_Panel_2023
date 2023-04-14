#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent): QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->setFixedSize(1280, 720);

    //class' initialization ======================================================================================================================================================================================
    connection = new Connection(this);
    arm = new Arm(this);
    drive = new Drive(this);
    joystickWidget = new JoystickWidget(this);
    joystickWidget->setGeometry(326, 150, 250, 250);

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
        for(int i=0; i<6; i++)
        {
            if(arm->buttonFunctionKeys[i].isActive == true) arm->activeButtonFunction = arm->buttonFunctionKeys[i].function;
        }
        arm->calculateSegmentsSpeeds(arm->containerX, arm->containerY, arm->containerZ, arm->containerPower, arm->activeButtonFunction, 120);
        //qDebug() <<arm->containerX<<arm->containerX/16383.0f;
        //arm->printButtonFunction(arm->activeButtonFunction);
        //arm->printButtonFunction(arm->buttonFunctionKeys[0].function);
        //arm->printButtonFunction(arm->buttonFunctionKeys[1].function);
    };

    auto armPhysicalLambda = [this]() {
        arm->calculateSegmentsSpeeds(0, 0, 0, 0, Arm::none, 120);
        updateUI(0, 0, 0, 0);

        for (int joystickNumber = 0; joystickNumber < numJoysticks; joystickNumber++) {
            if (ui->comboBox_arm_control->currentIndex() == joystickNumber + 1) {
                processJoystickState(joystickNumber, arm);
            }
        }
    };


    auto driveVirtualLambda =[this]()
    {
        drive->calculateWheelsSpeeds(joystickWidget->x() *16383, joystickWidget->y() *16383, ui->horizontalSlider_power_drive->value());
        //qDebug() <<joystickWidget->x()*16383 << joystickWidget->y()*16383 << ui->horizontalSlider_power_drive->value();
    };

    auto drivePhysicalLambda =[this]()
    {
        drive->calculateWheelsSpeeds(0, 0, 0);
        ui->horizontalSlider_power_drive->setValue(0);
        ui->horizontalSlider_x_drive->setValue(0);
        ui->horizontalSlider_y_drive->setValue(0);
        for (joystickNumber = 0; joystickNumber < numJoysticks; joystickNumber++)
        {
            if (ui->comboBox_drive_control->currentIndex() == joystickNumber + 1)
            {
                drive->containerX = (joystickState[joystickNumber].lX - physicalJoystickMaxValue / 2) / 2;
                drive->containerY = ((joystickState[joystickNumber].lY - physicalJoystickMaxValue / 2) / 2) *-1;
                drive->containerPower = ((joystickState[joystickNumber].rglSlider[0] / physicalJoystickMaxValue) *100 - 100) *-1;
                joystickPhysical[joystickNumber].poll(&joystickState[joystickNumber]);
                drive->calculateWheelsSpeeds(drive->containerX, drive->containerY, drive->containerPower);

                ui->horizontalSlider_x_drive->setValue(drive->containerX / 163.830);
                ui->horizontalSlider_y_drive->setValue(drive->containerY / 163.830);
                ui->horizontalSlider_power_drive->setValue(drive->containerPower);
                //qDebug() <<drive->containerX<<drive->containerX/16383.0f;
            }
        }
    };

    auto armControlVirtualLambda =[=]()
    {
        disconnect(connArmTimer);
        connArmTimer = connect(armTimer, &QTimer::timeout, arm, armVirtualLambda);
    };

    auto armControlPhysicalLambda =[=]()
    {
        disconnect(connArmTimer);
        connArmTimer = connect(armTimer, &QTimer::timeout, arm, armPhysicalLambda);
    };

    auto driveControlVirtualLambda =[=]()
    {
        disconnect(connDriveTimer);
        connDriveTimer = connect(driveTimer, &QTimer::timeout, drive, driveVirtualLambda);
    };

    auto driveControlPhysicalLambda =[=]()
    {
        disconnect(connDriveTimer);
        connDriveTimer = connect(driveTimer, &QTimer::timeout, drive, drivePhysicalLambda);
    };

    auto manageJoysticksPhysicalLambda =[this]()
    {
        numJoysticks = Joystick::deviceCount();
        ui->label_joysticks_count->setText(QString::number(numJoysticks));

        if (numJoysticks != joystickPhysical.size())
        {
            for (Joystick &joystick: joystickPhysical)
            {
                joystick.close();
            }

            joystickPhysical.clear();
            joystickPhysical.reserve(numJoysticks);
            for (int i = 0; i < numJoysticks; i++)
            {
                joystickPhysical.emplace_back(i);
                joystickPhysical[i].open();
            }
        }
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

//Drive with widgets------------------------------------------------------------------------------------------------------------------------------------------------------------------
connDriveTimer = connect(driveTimer, &QTimer::timeout, drive, driveVirtualLambda);
connect(drive, &Drive::controlVirtualJoystick, drive, driveControlVirtualLambda);

//Drive with joystick------------------------------------------------------------------------------------------------------------------------------------------------------------------
connect(drive, &Drive::controlPhysicalJoystick, drive, driveControlPhysicalLambda);

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
connect(arm, &Arm::controlPhysicalJoystick, arm, armControlPhysicalLambda);

//Joysticks management------------------------------------------------------------------------------------------------------------------------------------------------------------------
connJoystickPhysicalTimer = connect(joystickPhysicalTimer, &QTimer::timeout, manageJoysticksPhysicalLambda);

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
    updateButtonFunctionColors();

    });
//UI INITIALIZATION======================================================================================================================================================================================
ui->label_frames_sent_count->setStyleSheet("background-color: rgb(160,20,40);");
//UI Drive------------------------------------------------------------------------------------------------------------------------------------------------------------------
connect(joystickWidget, &JoystickWidget::xChanged, this, [=]()
{
    ui->horizontalSlider_x_drive->setValue(joystickWidget->x() *100);
    });
connect(joystickWidget, &JoystickWidget::yChanged, this, [=]()
{
    ui->horizontalSlider_y_drive->setValue(joystickWidget->y() *100);
    });
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
    else if (arg1 == "Physical Joystick 1" || arg1 == "Physical Joystick 2") emit drive->controlPhysicalJoystick();
}

void MainWindow::on_comboBox_arm_control_currentTextChanged(const QString &arg1)
{
    if (arg1 == "Virtual Sliders") emit arm->controlVirtualSliders();
    else if (arg1 == "Physical Joystick 1" || arg1 == "Physical Joystick 2") emit arm->controlPhysicalJoystick();
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

void MainWindow::updateButtonFunctionColors()
{
    if (arm->buttonFunctionKeys[0].isActive == true)
    {
        ui->pushButton_button_function_arm_1->setText("[1]");
    }
    else
    {
        ui->pushButton_button_function_arm_1->setText("1");
    }

    if (arm->buttonFunctionKeys[1].isActive == true)
    {
        ui->pushButton_button_function_arm_2->setText("[2]");
    }
    else
    {
        ui->pushButton_button_function_arm_2->setText("2");
    }

    if (arm->buttonFunctionKeys[2].isActive == true)
    {
        ui->pushButton_button_function_arm_3->setText("[3]");
    }
    else
    {
        ui->pushButton_button_function_arm_3->setText("3");
    }

    if (arm->buttonFunctionKeys[3].isActive == true)
    {
        ui->pushButton_button_function_arm_4->setText("[4]");
    }
    else
    {
        ui->pushButton_button_function_arm_4->setText("4");
    }

    if (arm->buttonFunctionKeys[4].isActive == true)
    {
        ui->pushButton_button_function_arm_5->setText("[5]");
    }
    else
    {
        ui->pushButton_button_function_arm_5->setText("5");
    }

    if (arm->buttonFunctionKeys[5].isActive == true)
    {
        ui->pushButton_button_function_arm_6->setText("[6]");
    }
    else
    {
        ui->pushButton_button_function_arm_6->setText("6");
    }
}

void MainWindow::updateUI(int containerX, int containerY, int containerZ, int containerPower) {
    ui->horizontalSlider_x_arm->setValue(containerX / 327.67 - 100);
    ui->horizontalSlider_y_arm->setValue(containerY / 327.67 - 100);
    ui->horizontalSlider_z_arm->setValue(containerZ / 327.67 - 100);
    ui->horizontalSlider_power_arm->setValue(containerPower);
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
}

void MainWindow::processJoystickState(int joystickNumber, Arm* arm) {
    arm->containerX = joystickState[joystickNumber].lX;
    arm->containerY = 65534 - joystickState[joystickNumber].lY;
    arm->containerZ = joystickState[joystickNumber].lRz;
    arm->containerPower = ((joystickState[joystickNumber].rglSlider[0] / physicalJoystickMaxValue) * 100 - 100) * -1;
    joystickPhysical[joystickNumber].poll(&joystickState[joystickNumber]);
    arm->buttonPressed = -1;

    for (int buttonPressedNow = 0; buttonPressedNow < 20; buttonPressedNow++) {
        if (joystickState[joystickNumber].rgbButtons[buttonPressedNow] & 0x80) {
            //qDebug() << buttonPressedNow;
            processButtonPressed(buttonPressedNow, arm);
        }
    }

    for (int i = 0; i < 6; i++) {
        if (arm->buttonFunctionKeys[i].isActive == true) {
            arm->activeButtonFunction = arm->buttonFunctionKeys[i].function;
        }
    }

    arm->calculateSegmentsSpeeds(arm->containerX, arm->containerY, arm->containerZ, arm->containerPower, arm->activeButtonFunction, arm->buttonPressed);
    updateUI(arm->containerX, arm->containerY, arm->containerZ, arm->containerPower);
}
