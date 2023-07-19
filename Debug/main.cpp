#include <QCoreApplication>
#include <QString>
#include <QThread>
#include <QDebug>
#include <db_lua.h>

#define DB_DELAY 1000

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);
    char word[50];
    DbLua *lua = new DbLua();

    while(1)
    {
        scanf("%s", word);
        QString input = word;
        QStringList commands;

        for( int i=0; i<input.size(); i++ )
        {
            if( input[i]=="r" )
            {
                commands << "resist";
            }
            else if( input[i]=="k" )
            {
                commands << "kick";
            }
            else if( input[i]=="f" )
            {
                commands << "fox";
            }
            else if( input[i]=="i" )
            {
                commands << "india";
            }
            else if( input[i]=="s" )
            {
                commands << "super";
            }
            else if( input[i]=="d" )
            {
                commands << "delta";
            }
            else if( input[i]=="-" )
            {
                commands << "dive";
            }
            else if( input[i]=="2" )
            {
                commands << "two";
            }
            else if( input[i]=="3" )
            {
                commands << "three";
            }
            else if( input[i]=="4" )
            {
                commands << "four";
            }
            else if( input[i]=="0" )
            {
                commands << "departure";
            }
            else
            {
            }
        }
        QThread::msleep(DB_DELAY);
        for( int i=0; i<commands.size(); i++ )
        {
            qDebug() << commands[i];
            lua->run(commands[i]);
        }
    }

    return a.exec();
}
