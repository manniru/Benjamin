#include "ch_char.h"
#include <unistd.h>
#include <QQmlProperty>
#include <QVariant>

ChChar::ChChar(QObject *ui,
               QObject *parent) : QObject(parent)
{
    root = ui;

    count_x   = QQmlProperty::read(root, "count_x").toInt();
    count_y   = QQmlProperty::read(root, "count_y").toInt();

    // Add numbers
    for( int i=0 ; i<10 ; i++ )
    {
        dictionary << 48+i;
    }
    // Add Charactars
    for( int i=0 ; i<26 ; i++ )
    {
        dictionary << 65+i;
    }
    // Add Special Charactor
//    dictionary << '\\';
    dictionary << ';';
    dictionary << '+';
    dictionary << '-';
    dictionary << '=';
    dictionary << '[';
    dictionary << ']';
    dictionary << '\'';
    dictionary << '.';
    dictionary << ',';
    dictionary << '/';

    createCells();
}

ChChar::~ChChar()
{
    ;
}

int ChChar::strToPos(QChar input)
{
    int char_id = input.toLatin1();
    return dictionary.indexOf(char_id);
}

void ChChar::addCell(QString name)
{
    QVariant         name_v(name);
    QGenericArgument name_arg = Q_ARG(QVariant, name_v);

    QMetaObject::invokeMethod(root, "addCell", name_arg);
}

void ChChar::createCells()
{
    for( int i=0 ; i<count_y ; i++ )
    {
        for( int j=0 ; j<count_x ; j++ )
        {
            QString cell_name = QString(dictionary[i]);
            if( dictionary[i]=='\\' )
            {
                cell_name = "\\\\";
            }
            else if( dictionary[i]=='\'' )
            {
                cell_name = "\\'";
            }

            if( dictionary[j]=='\\' )
            {
                cell_name += "\\\\";
            }
            else if( dictionary[j]=='\'' )
            {
                cell_name += "\\'";
            }
            else
            {
                cell_name += dictionary[j];
            }

            addCell(cell_name);
        }
    }
}
