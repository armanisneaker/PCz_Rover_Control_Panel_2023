#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QGraphicsView>
#include <vector>
#include <QDebug>
#include "mapmodule.h"
#include <cmath>

MapModule::MapModule(QObject *parent) : QGraphicsScene{parent}, choosen_index{0}
{
    setSceneRect(0,0,size_of_map,size_of_map);
    oldReferencePointX = newReferencePointX;
    oldReferencePointY = newReferencePointY;
    //setBackgroundBrush(QColor(35, 100, 161));

    QPen pen1;
    pen1.setColor(QColor(35, 100, 161, 255));
    pen1.setWidth(1000);

    QPen pen2;
    pen2.setColor(QColor(255,34,95,255));
    pen2.setWidth(1000);

    for(int i=0;i<size_of_map; i+= 10000)
    {
        if((i/10000)%10 == 0)
        {
            linia_pionowa.push_back(addLine(i,0,i,size_of_map, pen2));
            linia_pozioma.push_back(addLine(0,i,size_of_map,i, pen2));
        }
        else
        {
            linia_pionowa.push_back(addLine(i,0,i,size_of_map, pen1));
            linia_pozioma.push_back(addLine(0,i,size_of_map,i, pen1));
        }

        linia_pionowa[i/10000]->setEnabled(false);
        linia_pionowa[i/10000]->setVisible(false);

        linia_pozioma[i/10000]->setEnabled(false);
        linia_pozioma[i/10000]->setVisible(false);
    }



    rover.setPixmap(QPixmap(arrowPath));
    setBackgroundBrush(QBrush(QPixmap(main_path + "grid100px.png")));
    addItem(&rover);
    rover.setPos(QPointF(size_of_map/2 - rover.boundingRect().width()/2 ,size_of_map/2 - rover.boundingRect().height()/2));
}

MapModule::~MapModule()
{
    for(int i=0;i<linia_pionowa.size();i++)
    {
        delete linia_pionowa[i];
    }
}

QPointF MapModule::addPoint(double posX = 0.0000000000000, double posY = 0.0000000000000)
{
    //konwersja pozycji
    double distanceX = posX - newReferencePointX;
    double distanceY = posY - newReferencePointY;

    //rozmiar podawany w centymatrach 1px = 1cm
    int ConversionX = round((size_of_map/2) + (distanceX*KilometersForOneDegree*100000)); //*10000 ponieważ kilometry zmieniamy na centymetry
    int ConversionY = round((size_of_map/2) + (distanceY*KilometersForOneDegree*100000)); //*10000 ponieważ kilometry zmieniamy na centymetry

    qDebug() << "ConversionX: " << ConversionX << "   " << "ConversionY: " << ConversionY;
    item.push_back(new QGraphicsPixmapItem(QPixmap(redPointPath)));


    if(item[item.size()-1] == nullptr)
    {
        qDebug() << "Chuj w dupie a nie biznes";
    }
    else
    {
        if(item[item.size()-1] != nullptr)
        {
            item[item.size()-1]->setTransformOriginPoint(item[item.size()-1]->boundingRect().width()/2, item[item.size()-1]->boundingRect().height()/2);
            itemGroup.addToGroup(item[item.size()-1]);

            addItem(item[item.size()-1]);
            item[item.size()-1]->setPos(QPointF(ConversionX - (item[item.size()-1]->boundingRect().width()/2), ConversionY - (item[item.size()-1]->boundingRect().height()/2)));

            switch(scale_factor)
            {
                case scale_enum::m1:
                {
                    item[item.size()-1]->setScale(1);
                    break;
                }
                case scale_enum::m10:
                {
                    item[item.size()-1]->setScale(10);
                    break;
                }
                case scale_enum::m50:
                {
                    item[item.size()-1]->setScale(50);
                    break;
                }
                case scale_enum::m100:
                {
                    item[item.size()-1]->setScale(100);
                    break;
                }
                case scale_enum::m500:
                {
                    item[item.size()-1]->setScale(100);
                    break;
                }

                case scale_enum::m1000:
                {
                    item[item.size()-1]->setScale(500);
                    break;
                }

                default:
                {
                    break;
                }
            }
        }
        return QPointF(distanceX,distanceY);
    }
    return QPointF(0,0);
}

void MapModule::deleteCurrentPoint()
{
    removeItem(item[choosen_index]);
    itemGroup.removeFromGroup(item[choosen_index]);
    item.erase(item.begin() + choosen_index);
}

int MapModule::choosePoint(QPointF position,QTransform transform)
{

    QGraphicsItem * temp = itemAt(position,transform);

    if(temp == nullptr || temp == &rover)
    {
        return -1;
    }
    else
    {
        for(int i=0;item.size()-1;i++)
        {
            if(item[i] == temp)
            {
                change_colour(i);
                choosen_index = i;
                return i;
            }
        }
    }
    return -1;
}

void MapModule::getChoosenPoint(int newID)
{
    change_colour(newID);
    choosen_index = newID;


}

void MapModule::scale(scale_enum scale)
{

    scale_factor = scale;

    switch(scale)
    {
        case scale_enum::m1:
        {
            setBackgroundBrush(QBrush(QPixmap(main_path + "grid100px.png")));

            for(int i=0;i<item.size();i++)
            {
                item[i]->resetTransform();
                item[i]->setScale(1);
            }

            rover.resetTransform();
            rover.setScale(1);
            break;
        }

        case scale_enum::m10:
        {

            setBackgroundBrush(QBrush(QPixmap(main_path + "grid1000px.png")));

            for(int i=0;i<item.size();i++)
            {
                item[i]->resetTransform();
                item[i]->setScale(10);
            }

            rover.resetTransform();
            rover.setScale(10);
            break;
        }

        case scale_enum::m50:
        {
            setBackgroundBrush(QBrush(QPixmap(main_path + "grid5000px.png")));

            for(int i=0;i<item.size();i++)
            {
                item[i]->resetTransform();
                item[i]->setScale(50);
            }

            rover.resetTransform();
            rover.setScale(50);
            break;
        }

        case scale_enum::m100:
        {

            for(int i=0;i<item.size();i++)
            {
                item[i]->resetTransform();
                item[i]->setScale(100);
            }

            rover.resetTransform();
            rover.setScale(100);
            break;
        }

        case scale_enum::m500:
        {
            for(int i=0; i < linia_pionowa.size();i++)
            {
                linia_pionowa[i]->setVisible(false);
                linia_pozioma[i]->setVisible(false);
            }

            setBackgroundBrush(QBrush(QPixmap(main_path + "grid5000px.png")));
            for(int i=0;i<item.size();i++)
            {
                item[i]->resetTransform();
                item[i]->setScale(100);
            }

            rover.resetTransform();
            rover.setScale(100);
            break;

        }

        case scale_enum::m1000:
        {
            for(int i=0; i < linia_pionowa.size();i++)
            {
                linia_pionowa[i]->setVisible(true);
                linia_pozioma[i]->setVisible(true);
            }

            for(int i=0;i<item.size();i++)
            {
                item[i]->resetTransform();
                item[i]->setScale(500);
            }

            rover.resetTransform();
            rover.setScale(500);

            break;

        }

        default:
        {

            break;
        }
    }
}

void MapModule::change_colour(int idNewPoint)
{
    int real_posX = 0;
    int real_posY = 0;
    scale_enum scale = scale_factor;

    item[choosen_index]->setPixmap(QPixmap(redPointPath));
    item[idNewPoint]->setPixmap(QPixmap(greenPointPath));

    switch(scale)
    {
        case scale_enum::m1:
        {
            item[choosen_index]->setScale(1);
            item[idNewPoint]->setScale(1);
            break;
        }
        case scale_enum::m10:
        {
            item[choosen_index]->setScale(10);
            item[idNewPoint]->setScale(10);
            break;
        }
        case scale_enum::m50:
        {
            item[choosen_index]->setScale(50);
            item[idNewPoint]->setScale(50);
            break;
        }
        case scale_enum::m100:
        {
            item[choosen_index]->setScale(100);
            item[idNewPoint]->setScale(100);
            break;
        }
        case scale_enum::m500:
        {
            item[choosen_index]->setScale(100);
            item[idNewPoint]->setScale(100);
            break;
        }
    }



}

void MapModule::UpdatePosition(double temp_X, double temp_Y)//temp x i y są jako pozyce w gps więc najpierw konweraja na wspolrzedne mapy a potem reszta
{
    //aktualizacja reference point czyli także pozycję łazika (ale to osobno etykietka rovera to taki tylko feature)
    oldReferencePointX = newReferencePointX;
    oldReferencePointY = newReferencePointY;

    newReferencePointX = temp_X;
    newReferencePointY = temp_Y;

    double distanceX = temp_X - oldReferencePointX;
    double distanceY = temp_Y - oldReferencePointY;

    //rozmiar podawany w centymatrach 1px = 1cm
    int difference_X = distanceX*KilometersForOneDegree*100000; //*10000 ponieważ kilometry zmieniamy na centymetry
    int difference_Y  = distanceY*KilometersForOneDegree*100000; //*10000 ponieważ kilometry zmieniamy na centymetry


    qDebug() << "difference_X: "<< difference_X <<" difference_Y: "<< difference_Y;


    //przesunięcie wsztkich punktów względem środka (50000,50000) (w momencie kiedy to piszę 12.10.2022)
    //itemGroup.setPos(itemGroup.pos() + QPointF(-difference_X, -difference_Y));
    for(int i=0;i<item.size();i++)
    {
        item[i]->setPos(item[i]->pos() + QPointF(-difference_X, -difference_Y));
    }


    //tutaj taki twór bo przy prostszym zapisie wywalało mi błędy idk why ale windows mnie już powoli wkuriwa
    QList<QGraphicsView *> lista = views();
    QGraphicsView * view = lista.at(0);
    view->centerOn(QPointF(size_of_map/2,size_of_map/2));
}
