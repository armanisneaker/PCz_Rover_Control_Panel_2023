#ifndef MAPMODULE_H
#define MAPMODULE_H

#include <QObject>
#include <QGraphicsScene>
#include <QGraphicsItem>
#include <QGraphicsItemGroup>
#include <QDir>

#include <vector>

#include "structures.h"
#include "rover_pointer.h"

class MapModule : public QGraphicsScene
{
    Q_OBJECT
public:
    explicit MapModule(QObject *parent = nullptr);
    ~MapModule();

    //ustawia punkt odniesienia czyli początkowa pozycja łazika
    //punkty są umieszczane w danej odległości od punktu odniesienia
    //który non stop się zmienia (łazik na mapie jest nieruchimo a wszytskie punkty wokół niego się poruszają)
    //szerokość "wizji" to 10km na stopnie to 0.0898315658 (przy założeniu że ziemia to super idealna
    //kula a jej obwód to 40075km

    //stworzenie/ododanie puntku
    QPointF addPoint(double , double );

    //usuniecie punktu
    void deleteCurrentPoint();

    //wybranie punku(gdy kliknięte na mape) przekazanie pozycji kursora do mapmodule
    int choosePoint(QPointF, QTransform);

    //odebranie zacznaczonego punktu(gdy kliknięte na listę)
    void getChoosenPoint(int currentPoint);

    //odpowiada za zmianę tekstur siatki oraz objektów względem przybliżenia
    //wartości podoawane jako wartosci pikseli jako enumy m1 m10 m50 m100 m500
    void scale(scale_enum);

    const int size_of_map = 1000000; //1px to 1cm więc mapa pokazuje obszar 10kmx10km przy przybliżeniu

    void UpdatePosition(double, double); //zmienia pozycję wszytskich punktów o różnicę w odległości punktów
    std::vector<QGraphicsPixmapItem *> item;
    QGraphicsItemGroup itemGroup;
    rover_pointer rover;

signals:



private:

    void change_colour(int);
    double oldReferencePointX = 10.000000000000;
    double newReferencePointX = 10.000000000000;
    double oldReferencePointY = 10.000000000000;
    double newReferencePointY = 10.000000000000;
    const double KilometersForOneDegree = 111.319444444444;
    scale_enum scale_factor = scale_enum::m1;
    int choosen_index = 0;

    QString main_path =  "./images/";
    QString redPointPath = main_path + ("redPoint.png");
    QString greenPointPath = main_path + ("greenPoint.png");
    QString arrowPath = main_path + ("arrow.png");

    std::vector<QGraphicsLineItem *> linia_pionowa;
    std::vector<QGraphicsLineItem *> linia_pozioma;


};

#endif // MAPMODULE_H
