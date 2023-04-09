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

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;

    Connection* connection;
    Drive* drive;
    Arm* arm;
    JoystickWidget* joystickWidget;
    std::vector<Joystick> joystickPhysical;

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
    QMetaObject::Connection connDriveTimer;
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
