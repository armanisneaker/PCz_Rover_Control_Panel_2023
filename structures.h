#ifndef STRUCTURES_H
#define STRUCTURES_H

#include <QObject>
#include <QGraphicsItem>
#include <QPoint>
#include <QTableWidgetItem>

enum class scale_enum
{
    m1 = 1,
    m10 = 10,
    m50 = 50,
    m100 = 100,
    m500 = 500,
    m1000 = 1000
};

struct Movement
{
    int Value[4]{};    //power, x, y, z
    int Combobox[6]{};  //No to akurat jest fajna zabawa xd gdzieś dam listę tego gówna co siedzi w środku xdxd
};

struct Fast_Settings
{
    int clicked = 0;
    std::string combobox[3];
};


struct Navigation
{
    int index_number = 0;
    int choosen_index = 0;
    std::vector<QTableWidgetItem *> itemID;
    std::vector<QTableWidgetItem *> itemRow;
    std::vector<QTableWidgetItem *> itemColumn;
    std::vector<double> pos_X;
    std::vector<double> pos_Y;
};

struct JoyStick
{
    QPoint point;
    int start_pos_X;
    int start_pos_Y;
    int mouse_pos_X;
    int mouse_pos_Y;
    bool ispressed = false;

    int diff_X;
    int diff_Y;
};

struct MouseEvent
{
    int pos_x;
    int pos_y;
};

struct Scale
{
    QTransform viewtransform;
    scale_enum scale;

};





#endif // STRUCTURES_H
