#include "ch_processor_w.h"
#include <unistd.h>

ChProcessorW::ChProcessorW(QObject *ui,
                           QObject *parent) : QObject(parent)
{
    root = ui;

    count_x   = QQmlProperty::read(root, "count_x").toInt();
    count_y   = QQmlProperty::read(root, "count_y").toInt();
    meta_mode = 0;
    drag_mode = 0;
    click_mode = CH_LEFT_CLICK;

    exec = new ChExecW(root);
}

ChProcessorW::~ChProcessorW()
{
    ;
}

void ChProcessorW::showUI(QString text)
{
//    reset();
    exec->updateScreen(text);
    if( text=="no_click" )
    {
        click_mode = CH_NO_CLICK;
    }
    else if( text=="side" )
    {
        click_mode = CH_RIGHT_CLICK;
    }
    else if( text=="double" )
    {
        click_mode = CH_DOUBLE_CLICK;
    }
    else if( text=="comment" )
    {
        click_mode = CH_LEFT_CLICK;
    }
    else if( text=="persist" )
    {
        click_mode = CH_PERSIST;
    }
    else if( text=="drag" )
    {
        click_mode = CH_DRAG;
        qDebug() << "showUI: DRAG";
    }
    else
    {
        qDebug() << "showUI: Unknown" << text;
        click_mode = CH_LEFT_CLICK;
    }

    QQmlProperty::write(root, "opacity", CHESS_MAX_OPACITY);
    QQmlProperty::write(root, "visible", true);
    QQmlProperty::write(root, "ch_timer", true);
    meta_mode = 0;

//    exec->activateWindow();
    exec->setLanguage();
}

void ChProcessorW::hideUI()
{
    QQmlProperty::write(root, "visible", 0);
    QQmlProperty::write(root, "ch_buffer", "");
    QQmlProperty::write(root, "ch_cell_color", "#cf000000");
    QMetaObject::invokeMethod(root, "resetHighlight");
    key_buf = "";

    if( click_mode==CH_LEFT_CLICK )
    {
        QThread::msleep(50);

        if( drag_mode )
        {
            exec->mouseRelease(1);
            drag_mode = 0;
        }
        else
        {
            exec->sendMouseKey(1);
        }
        meta_mode = 0;
        click_mode = CH_LEFT_CLICK;
    }
    else if( click_mode==CH_RIGHT_CLICK )
    {
        QThread::msleep(50);
        exec->sendMouseKey(3);
        meta_mode = 0;
        click_mode = CH_LEFT_CLICK;
    }
    else if( click_mode==CH_DOUBLE_CLICK )
    {
        QThread::msleep(50);
        exec->sendMouseKey(1);
        QThread::msleep(50);
        exec->sendMouseKey(1);
        meta_mode = 0;
        click_mode = CH_LEFT_CLICK;
    }
    else if( click_mode==CH_PERSIST )
    {
        QThread::msleep(50);
        exec->sendMouseKey(1);
        QThread::msleep(10);
        QQmlProperty::write(root, "visible", 1);
        meta_mode = 0;
//        exec->activateWindow();
        QQmlProperty::write(root, "ch_timer", true);
    }
    else if( click_mode==CH_DRAG )
    {
        QThread::msleep(50);
        exec->mousePress(1);
        meta_mode = 0;
        drag_mode = 1;
        click_mode = CH_LEFT_CLICK;
        QThread::msleep(50);
        QQmlProperty::write(root, "visible", 1);
    }
}

// Escape Pressed
void ChProcessorW::cancel()
{
    click_mode = CH_NO_CLICK;
    hideUI();
    ch_returnFocus();
}

// enable finer detail mode
void ChProcessorW::meta()
{
    QQmlProperty::write(root, "opacity", CHESS_MAX_OPACITY);
    meta_mode = 1; ///FIXME: MAKE THIS TOGGLE
    QMetaObject::invokeMethod(root, "metaMode");
}

void ChProcessorW::key(int val)
{
    qDebug() << "key" << val;
    QQmlProperty::write(root, "opacity", CHESS_MAX_OPACITY);
    if( val==VK_BACK )
    {
        if( key_buf.length()>0 )
        {
            QMetaObject::invokeMethod(root, "backspace");
            key_buf.remove( key_buf.length()-1, 1 );
        }
    }
    else if( val>CH_KEY_MIN && val<CH_KEY_MAX )
    {
        processNatoKey(val);
    }
}

void ChProcessorW::processNatoKey(int key)
{
    key_buf += (char)key;
    QQmlProperty::write(root, "ch_timer", false);

    QVariant key_v(key);
    QGenericArgument arg_key = Q_ARG(QVariant, key_v);
    QMetaObject::invokeMethod(root, "keyHandler", arg_key);
    if( key_buf.length()==CHESS_CHAR_COUNT )
    {
        int x, y;
        strToPos(key_buf, &x, &y);
        setPos(x, y);
        if( meta_mode==0 )
        {
            hideUI();
        }
    }
    else if( key_buf.length()==CHESS_CHAR_COUNT+1 &&
             meta_mode )
    {
        if( '9'<key || key<'0' )
        {
            qDebug() << "wrong fine input";
            key_buf.remove( key_buf.length()-1, 1 ); // remove last char
            return;
        }
        setPosFine(key);
        hideUI();
    }
}

void ChProcessorW::strToPos(QString input, int *x, int *y)
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

void ChProcessorW::setPos(int x, int y)
{
    // window width, height, x and y
    int w_width  = QQmlProperty::read(root, "width").toInt();
    int w_height = QQmlProperty::read(root, "height").toInt();
    int w_x      = QQmlProperty::read(root, "x").toInt();
    int w_y      = QQmlProperty::read(root, "y").toInt();
//    qDebug() << w_height << w_x << w_y;
    double width  = w_width /(count_x-0.1);
    double height = w_height/(count_y-0.1);

    x = w_x + qRound(x*width  + width /2);
    y = w_y + qRound(y*height + height/2);

    SetCursorPos(x, y);
}

void ChProcessorW::setPosFine(int key)
{
    if( '9'<key || key<'1' )
    {
        qDebug() << "wrong fine input";
        return;
    }
    int input = key-'1';
    qDebug() << input;
    int x = input%3;
    int y = input/3;

    POINT pt;
    GetCursorPos(&pt);

    int r_x = (x - 1) * 20;
    int r_y = (y - 1) * 20;

    int x2 = pt.x + r_x;
    int y2 = pt.y + r_y;

    SetCursorPos(x2, y2);
}

