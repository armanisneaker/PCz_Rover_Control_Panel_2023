#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QElapsedTimer>
#include <vector>
#include "arm.h"
#include "connection.h"
#include "drive.h"
#include "joystickwidget.h"
#include "joystickdirectinput.h"
#include "ui_mainwindow.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void updateButtonFunctionColors();

    void updateUI(int containerX, int containerY, int containerZ, int containerPower);
    void processButtonPressed(int buttonPressedNow, Arm* arm);
    void processJoystickState(int joystickNumber, Arm* arm);


private:
    Ui::MainWindow *ui;

    Connection* connection;
    Drive* drive;
    Arm* arm;
    JoystickWidget* joystickWidget;
    DirectInputJoystick* joystick;


    QTimer *connectionTimer;
    QTimer *armTimer;
    QTimer *driveTimer;
    QTimer *uiTimer;
    QTimer* frameStatusTimer;
    QTimer* joystickPhysicalTimer;
    QElapsedTimer* lastFrameSentTime;

    int timeSinceLastFrameSent;

    QMetaObject::Connection connConnectionTimer;
    QMetaObject::Connection connArmTimer;
    QMetaObject::Connection connArmPhysicalX;
    QMetaObject::Connection connArmPhysicalY;
    QMetaObject::Connection connArmPhysicalZ;
    QMetaObject::Connection  connArmPhysicalPower;

    QMetaObject::Connection connDriveTimer;
    QMetaObject::Connection connDrivePhysicalX;
    QMetaObject::Connection connDrivePhysicalY;
    QMetaObject::Connection connDrivePhysicalZ;
    QMetaObject::Connection connDrivePhysicalPower;

    QMetaObject::Connection connJoystickPhysicalTimer;
    QMetaObject::Connection connDriveControlVirtual;
    QMetaObject::Connection connDriveControlPhysical;

    DIJOYSTATE2 joystickState[4];
    unsigned int numJoysticks = 0;
    unsigned int numJoysticksOld = 0;
    int joystickNumber;
    float physicalJoystickMaxValue = 65535;

private slots:
    void updateFrameStatusLabel();
    void on_comboBox_drive_control_currentTextChanged(const QString &arg1);
    void on_comboBox_arm_control_currentTextChanged(const QString &arg1);
    void on_horizontalSlider_x_arm_sliderReleased();
    void on_horizontalSlider_y_arm_sliderReleased();
    void on_horizontalSlider_z_arm_sliderReleased();
    void on_horizontalSlider_x_drive_sliderReleased();
    void on_horizontalSlider_y_drive_sliderReleased();
};
#endif // MAINWINDOW_H
