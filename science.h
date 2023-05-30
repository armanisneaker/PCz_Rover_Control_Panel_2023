#ifndef SCIENCE_H
#define SCIENCE_H

#include <QObject>
#include <QIODevice>

class Science : public QObject
{
    Q_OBJECT
public:
    Science();

    bool lid1;
    bool lid2;
    bool lid3;
    bool lid4;

    bool pump1;
    bool pump2;
    bool pump3;
    bool pump4;

    int16_t revolverPosition;

    void changeRevolverPosition(bool direction);

    void calculateScienceValues();

    QByteArray frame;

signals:
    void revolverPositionChanged();
};

#endif // SCIENCE_H
