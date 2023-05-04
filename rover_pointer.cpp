#include "rover_pointer.h"

rover_pointer::rover_pointer()
{
}

void rover_pointer::rotate(int temp_angle)
{
    angle = temp_angle;
    setRotation(angle);

}

bool rover_pointer::setPixmap(QPixmap pixmap)
{
    QGraphicsPixmapItem::setPixmap(pixmap);
    setTransformOriginPoint(QPointF(pixmap.width()/2, pixmap.height()/2));

    if(QGraphicsPixmapItem::pixmap().isNull())
    {
       return false;
    }

    return true;
}
