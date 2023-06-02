#include "mm_channel.h"
#include <unistd.h>
#include <QApplication>
#include <QDebug>

MmChannel::MmChannel(QObject *parent) : QObject(parent)
{
    createPipe();
}

MmChannel::~MmChannel()
{
    ;
}

void MmChannel::listenPipe()
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

void MmChannel::processLine(QString line)
{
    line = line.trimmed();
    QStringList fields = line.split(MM_PIPE_SEP,
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

void MmChannel::createPipe()
{
    // To create an instance of a named pipe by using CreateNamedPipe,

    // the user must have FILE_CREATE_PIPE_INSTANCE access to the named pipe object.
    hPipe = CreateNamedPipe(TEXT(MM_PIPE_PATH),
                            PIPE_ACCESS_INBOUND,            // dwOpenMode. The flow of data in the pipe goes from client to server only
                            PIPE_TYPE_BYTE | PIPE_WAIT,     // dwPipeMode
                            1,                              // nMaxInstances
                            BUFFER_SIZE,                    // nOutBufferSize
                            BUFFER_SIZE,                    // nInBufferSize
                            NMPWAIT_WAIT_FOREVER,           // nDefaultTimeOut
                            nullptr);                       // lpSecurityAttributes

    if( hPipe==INVALID_HANDLE_VALUE )
    {
        qDebug(MM_PIPE_PATH"Failed");
    }
    qDebug() << MM_PIPE_PATH << "pipe Created";
}

void MmChannel::processCommand(QString cmd, QString arg)
{
    qDebug() << "pro" << cmd;
    if( cmd=="Meta" )
    {
        emit meta(arg);
    }
    else if( cmd.contains("Sys_") )
    {
        cmd.remove(0, 4);
//        int key_val = cmd.toInt();

        qDebug() << "pc" << cmd;
        emit meta(cmd);
    }
    else
    {
        //emit show_chess(cmd);
    }
    QCoreApplication::processEvents();
    QGuiApplication::processEvents();
}
