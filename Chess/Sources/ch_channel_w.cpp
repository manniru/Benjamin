#include "ch_channel_w.h"
#include <unistd.h>
#include <QApplication>

ChChannelW::ChChannelW(QObject *parent) : QObject(parent)
{
    createPipe();
}

ChChannelW::~ChChannelW()
{
    ;
}

void ChChannelW::listenPipe()
{
    qDebug() << "listenPipe";
    char buffer[BUFFER_SIZE];
    DWORD dwRead;
    while( hPipe!=INVALID_HANDLE_VALUE )
    {
        // wait for someone to connect to the pipe
        if( ConnectNamedPipe(hPipe, nullptr)!=FALSE )
        {
            qDebug() << "Connect client";
            while( ReadFile(hPipe, buffer, sizeof(buffer)-1, &dwRead, nullptr)!=FALSE )
            {
                // add terminating zero
                buffer[dwRead] = 0;
                QString input(buffer);

                QStringList lines = input.split("\n", QString::SkipEmptyParts);

                for( int i=0 ; i<lines.length() ; i++ )
                {
                    processLine(lines[i]);
                }
            }
        }
        else
        {
            qDebug() << "failed";
        }

        qDebug() << "Client Disconnected";
        DisconnectNamedPipe(hPipe);
    }
}

void ChChannelW::processLine(QString line)
{
    line = line.trimmed();
    QStringList fields = line.split(CH_NP_SEPARATOR,
                               QString::SkipEmptyParts);

    QString cmd = fields[0];
    QString arg;
    if( fields.length()>1 )
    {
        arg = fields[1];
    }
    qDebug() << "Command:" << cmd
             << "Args:" << arg;

    processCommand(cmd, arg);
}

void ChChannelW::createPipe()
{
    // To create an instance of a named pipe by using CreateNamedPipe,
    // the user must have FILE_CREATE_PIPE_INSTANCE access to the named pipe object.
    hPipe = CreateNamedPipe(TEXT(CH_PIPE_PATH),
                            PIPE_ACCESS_INBOUND,            // dwOpenMode. The flow of data in the pipe goes from client to server only
                            PIPE_TYPE_BYTE | PIPE_WAIT,     // dwPipeMode
                            1,                              // nMaxInstances
                            BUFFER_SIZE,                    // nOutBufferSize
                            BUFFER_SIZE,                    // nInBufferSize
                            NMPWAIT_WAIT_FOREVER,           // nDefaultTimeOut
                            nullptr);                       // lpSecurityAttributes

    if( hPipe==INVALID_HANDLE_VALUE )
    {
        qDebug(CH_PIPE_PATH"Failed");
    }
    qDebug() << CH_PIPE_PATH << "pipe Created";
}

void ChChannelW::processCommand(QString cmd, QString arg)
{
    qDebug() << "pro" << cmd;
    if( cmd=="show" )
    {
        emit show_chess(arg);
    }
    else if( cmd=="Meta" )
    {
        emit meta();
    }
    else if( cmd.contains("Key_") )
    {
        cmd.remove(0, 4);
        int key_val = cmd.toInt();

        if( key_val==VK_ESCAPE )
        {
            emit cancel();
        }
        else
        {
            qDebug() << "pc" << key_val;
            emit key_chess(key_val);
        }
    }
    else
    {
        emit show_chess(cmd);
    }
    QCoreApplication::processEvents();
    QGuiApplication::processEvents();
}
