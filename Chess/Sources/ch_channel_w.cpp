#include "ch_channel_w.h"
#include <unistd.h>

ChChannelW::ChChannelW(QObject *ui,QObject *parent) : QObject(parent)
{
    ConnectDBus();
    root = ui;

    count_x   = QQmlProperty::read(root, "count_x").toInt();
    count_y   = QQmlProperty::read(root, "count_y").toInt();
    meta_mode = 0;
    click_mode = CH_LEFT_CLICK;
}

ChChannelW::~ChChannelW()
{
    ;
}

void ChChannelW::ListenPipe()
{
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

                for(int i=0 ; i<lines.length() ; i++)
                {
                    processLine(lines[i]);
                }
            }
        }

        qDebug() << "Client Disconnected";
        DisconnectNamedPipe(hPipe);
    }
}

void ChChannelW::processLine(QString line)
{
    line = line.trimmed();
    QStringList fields = line.split(RE_NP_SEPARATOR,
                               QString::SkipEmptyParts);
    if ( fields.length()!=2 )
    {
        qDebug() << "Error 128: wrong field length, `"
                    + line + "`";
        return;
    }

    QString key_type = fields[0];
    QString key_code = fields[1];
    qDebug() << "Command:" << key_type
             << "Args:" << key_code;

    processCommand(key_type, key_code);
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
    if( cmd=="show" )
    {
        emit show(arg);
    }
    else if( cmd=="side" )
    {
        emit side(arg);
    }
}
