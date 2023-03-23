#include "ab_console_controller.h"
#include <QDebug>

AbConsoleController::AbConsoleController(int mode, QObject *parent) : QObject(parent)
{
    line_number = 0;
    flag = mode;

    commands << "./init.sh";
    commands << "./train.sh";
}

AbConsoleController::~AbConsoleController()
{

}

void AbConsoleController::run()
{
    DWORD read_len;
    char chBuf[CONSOLE_BUF_SIZE];
    BOOL ok;
    QString output;

    while( true )
    {
        ok = ReadFile(handle, chBuf,
                      CONSOLE_BUF_SIZE, &read_len, NULL);
        if( !ok || read_len==0 )
        {
            qDebug() << flag << "ERROR" << ok << read_len;
            break;
        }
        chBuf[read_len] = 0;

        output = chBuf;
        emit readyData(output, flag);
        qDebug() << flag << output;
        processLine(output);
    }
}

void AbConsoleController::processLine(QString line)
{
    if( line.contains("Arch>") )
    {
        if( line_number<commands.length() )
        {
            QString cmd = "KalB.exe run ";
            cmd += commands[line_number] + "\n";
            line_number++;
            emit sendCommand(cmd);
        }
    }
}
