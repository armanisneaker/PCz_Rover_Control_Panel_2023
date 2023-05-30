#include "science.h"

Science::Science()
{
    lid1 = false;
    lid2 = false;
    lid3 = false;
    lid4 = false;

    pump1 = false;
    pump2 = false;
    pump3 = false;
    pump4 = false;

    revolverPosition = 0;

    calculateScienceValues();
}

void Science::changeRevolverPosition(bool direction)
{
    if(direction)
    {
        revolverPosition += 45;
        if(revolverPosition >= 360)
        {
            revolverPosition = 0;
        }
    }
    else
    {
        revolverPosition -= 45;
        if(revolverPosition < 0)
        {
            revolverPosition = 315;
        }
    }
}

void Science::calculateScienceValues()
{
    QDataStream stream(&frame, QIODevice::WriteOnly);
    stream << lid1 << lid2 << lid3 << lid4 << pump1 << pump2 << pump3 << pump4 << revolverPosition;
    emit revolverPositionChanged();
}
