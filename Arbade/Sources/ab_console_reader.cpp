#include "ab_console_reader.h"
#include <QDebug>

AbConsoleReader::AbConsoleReader(int mode, QObject *parent) : QObject(parent)
{
    state = 0;
    flag = mode;
}

AbConsoleReader::~AbConsoleReader()
{

}

void AbConsoleReader::run()
{
    DWORD read_len;
    char chBuf[CONSOLE_BUF_SIZE];
    BOOL ok;
    QString output;

    while(true)
    {
        ok = ReadFile(handle, chBuf,
                      CONSOLE_BUF_SIZE, &read_len, NULL);
        if( !ok || read_len==0 )
        {
            qDebug() << flag << "ERROR" << ok << read_len;
            break;
        }

        output = chBuf;
        qDebug() << flag << output;
        processLine(output);
        emit readyData(output, flag);
    }
}

void AbConsoleReader::processLine(QString line)
{
    if( line.contains("Arch>") )
    {
        if( state==0 )
        {
            QString cmd = "KalB.exe\n";
    //        QString cmd = "dir\n";
        //    QString cmd = "ls\n";
            state = 1;
            emit sendCommand(cmd);
        }
    }
}
