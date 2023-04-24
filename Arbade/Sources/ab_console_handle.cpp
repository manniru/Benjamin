#include "ab_console_handle.h"
#include <QDebug>
#include <QDir>
#include <QThread>

AbConsoleHandle::AbConsoleHandle(int fl, QObject *parent) : QObject(parent)
{
    flag = fl;
}

AbConsoleHandle::~AbConsoleHandle()
{
    ;
}

void AbConsoleHandle::readData()
{
    DWORD read_len;
    char chBuf[CONSOLE_BUF_SIZE];
    BOOL ok;
    QString output;

    while( true )
    {
        ok = ReadFile(handle, chBuf,
                      CONSOLE_BUF_SIZE, &read_len, NULL);
        if( ok==0 || read_len==0 )
        {
            qDebug() << "ERROR" << ok << read_len;
            break;
        }
        chBuf[read_len] = 0;

        output = chBuf;
        // to support clink
        output = output.replace("\b", "");
        emit readyData(output, flag);
//        qDebug() << "hi" << flag << output;
    }
    qDebug() << "THREAD CRASHED";
}
