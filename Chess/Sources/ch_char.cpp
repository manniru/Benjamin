#include "ch_char.h"
#include <unistd.h>

ChChar::ChChar(QObject *ui,
               QObject *parent) : QObject(parent)
{
    root = ui;

    count_x   = QQmlProperty::read(root, "count_x").toInt();
    count_y   = QQmlProperty::read(root, "count_y").toInt();
}

ChChar::~ChChar()
{
    ;
}

void ChChar::strToPos(QString input, int *x, int *y)
{
    char ch_x = input.toStdString()[1];
    if( '0'<=ch_x && ch_x<='9' )
    {
        *x = (int)ch_x - '0';
    }
    else
    {
        *x = (int)ch_x - 'A' + 10;
    }

    char ch_y = input.toStdString()[0];
    if( '0'<=ch_y && ch_y<='9' )
    {
        *y = (int)ch_y - '0';
    }
    else
    {
        *y = (int)ch_y - 'A' + 10;
    }
}

void ChChar::addCell(QString name)
{

}
