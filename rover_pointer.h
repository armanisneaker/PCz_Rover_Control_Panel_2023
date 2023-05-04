#ifndef ROVER_POINTER_H
#define ROVER_POINTER_H

#include <QObject>
#include <QGraphicsPixmapItem>

class rover_pointer : public QObject, public QGraphicsPixmapItem
{
    Q_OBJECT
public:
    explicit rover_pointer();
    void rotate(int);//obraca się na wskazny kąt (nie o podany tylko na wskazany)
    bool setPixmap(QPixmap pixmap);
signals:

private:
    int angle = 0;


};

#endif // ROVER_POINTER_H
