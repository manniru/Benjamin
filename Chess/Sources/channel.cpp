#include "channel.h"
#include <unistd.h>

Channel::Channel(QObject *ui,QObject *parent) : QObject(parent)
{
    ConnectDBus();
    root = ui;
}

Channel::~Channel()
{
    ;
}

void Channel::ConnectDBus()
{
    QDBusConnection session = QDBusConnection::sessionBus();

    if (!session.isConnected())
    {
        qFatal("Cannot connect to the D-Bus session bus.");
        return;
    }

    session.connect("", "/", COM_NAME, "show", this, SLOT(showUI(const QString &)));

    if(!session.registerService(COM_NAME)) {
        qFatal("Another session is on DBus.");
        // This cannot be automatic because killing assistant also kill
        // this instant too
        return;
    }
}

void Channel::showUI(const QString &text)
{
//    reset();
    QQmlProperty::write(root, "visible", 1);
}

void Channel::reset()
{
    QQmlProperty::write(root, "visible", 0);
    QQmlProperty::write(root, "ch_buffer", "");
    QMetaObject::invokeMethod(root, "resetHighlight");
    key_buf = "";
}

void Channel::keyPressed(int key)
{
    if( key==CH_BACKSPACE_CODE )
    {
        if( key_buf.length()>0 )
        {
            key_buf.remove( key_buf.length()-1, 1 );
        }
    }

    if( key>CH_KEY_MIN && key<CH_KEY_MAX )
    {
        key_buf += (char)key;
        if( key_buf.length()==CHESS_CHAR_COUNT )
        {
            int x, y;
            strToPos(key_buf, &x, &y);
            setPos(x, y);
            reset();
        }
    }
}

void Channel::strToPos(QString input, int *x, int *y)
{
    *x = (int)(input.toStdString()[1]) - 'A';
    *y = (int)(input.toStdString()[0]) - 'A';
}

void Channel::setPos(int x, int y)
{
    double width = 74.1;
    double height = 41.85;
    QString cmd = "xdotool mousemove ";
    cmd += QString::number(qRound(x*width + width/2));
    cmd += " ";
    cmd += QString::number(qRound(y*height+height/2));

    system(cmd.toStdString().c_str());

}
