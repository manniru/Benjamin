#include "ab_console_reader.h"
#include <QDebug>

AbConsoleReader::AbConsoleReader(QObject *parent) : QObject(parent)
{

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
        qDebug() << "ye khoruji";
        ok = ReadFile(handle, chBuf,
                      CONSOLE_BUF_SIZE, &read_len, NULL);
        qDebug() << "do khoruji";
        if( !ok || read_len==0 )
        {
            qDebug() << "se khoruji" << ok << read_len;
            break;
        }

        output = chBuf;
        qDebug() << output;
        processLine(output);
        emit readyData(output);
    }
}

void AbConsoleReader::processLine(QString line)
{
    if( line.contains("Arch>") )
    {
        emit readyCommand();
    }
}
