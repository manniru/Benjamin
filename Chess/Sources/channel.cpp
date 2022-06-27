#include "channel.h"
#include <unistd.h>

Channel::Channel(QObject *ui,QObject *parent) : QObject(parent)
{
    ConnectDBus();
    root = ui;

    count_x = QQmlProperty::read(root, "count_x").toInt();
    count_y = QQmlProperty::read(root, "count_y").toInt();
}

Channel::~Channel()
{
    ;
}

void Channel::ConnectDBus()
{
    QDBusConnection session = QDBusConnection::sessionBus();

    if( !session.isConnected() )
    {
        qFatal("Cannot connect to the D-Bus session bus.");
        return;
    }

    session.connect("", "/", COM_NAME, "show", this, SLOT(showUI(const QString &)));

    if( !session.registerService(COM_NAME) )
    {
        return;
    }
}

void Channel::showUI(const QString &text)
{
//    reset();
    system("Scripts/disable_picom_dim.sh");
    QQmlProperty::write(root, "visible", 1);
    QQmlProperty::write(root, "y", 500);
}

void Channel::reset()
{
    QQmlProperty::write(root, "visible", 0);
    QQmlProperty::write(root, "ch_buffer", "");
    QMetaObject::invokeMethod(root, "resetHighlight");
    key_buf = "";
    system("Scripts/enable_picom_dim.sh");
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
    char ch_x = input.toStdString()[1];

    if( '0'<=ch_x && ch_x<='9' )
    {
        *x = (int)ch_x - '0';
    }
    else
    {
        *x = (int)ch_x - 'A' + 10;
    }

    *y = (int)(input.toStdString()[0]) - 'A';
}

void Channel::setPos(int x, int y)
{
    // window width, height, x and y
    int w_width  = QQmlProperty::read(root, "width").toInt();
    int w_height = QQmlProperty::read(root, "height").toInt();
    int w_x      = QQmlProperty::read(root, "x").toInt();
    int w_y      = QQmlProperty::read(root, "y").toInt();
    qDebug() << w_height << w_x << w_y;
    double width  = w_width /(count_x-0.1);
    double height = w_height/(count_y-0.1);

    x = w_x + qRound(x*width  + width /2);
    y = w_y + qRound(y*height + height/2);
    QString cmd = "xdotool mousemove ";
    cmd += QString::number(x);
    cmd += " ";
    cmd += QString::number(y);

    system(cmd.toStdString().c_str());
}
