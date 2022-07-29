#include "ch_processor_w.h"
#include <unistd.h>
#include <QWindow>

ChProcessorW::ChProcessorW(ChChannelW *ch, QObject *ui,
                           QObject *parent) : QObject(parent)
{
    root = ui;

    count_x   = QQmlProperty::read(root, "count_x").toInt();
    count_y   = QQmlProperty::read(root, "count_y").toInt();
    meta_mode = 0;
    click_mode = CH_LEFT_CLICK;

    connect(ch, SIGNAL(show(QString)),
            this, SLOT(showUI(QString)));

    QWindow *window = qobject_cast<QWindow *>(ui);
    hWnd = (HWND)(window->winId());
}

ChProcessorW::~ChProcessorW()
{
    ;
}

void ChProcessorW::showUI(QString text)
{
//    reset();
    if( text=="no_click" )
    {
        click_mode = CH_NO_CLICK;
    }
    else  if( text=="side" )
    {
        click_mode = CH_RIGHT_CLICK;
    }
    else
    {
        click_mode = CH_LEFT_CLICK;
    }

    QQmlProperty::write(root, "opacity", CHESS_MAX_OPACITY);
    QQmlProperty::write(root, "visible", true);
    QQmlProperty::write(root, "ch_timer", true);

    activateWindow();
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
        sendLeftKey();
    }
    else if( click_mode==CH_RIGHT_CLICK )
    {
        QThread::msleep(50);
        sendRightKey();
    }
    meta_mode = 0;
    click_mode = CH_LEFT_CLICK;
}

void ChProcessorW::createStatFile()
{
//    system("touch chess_en");
}

void ChProcessorW::rmStatFile()
{
//    system("rm chess_en");
}

void ChProcessorW::keyPressed(int key)
{
    qDebug() << "key" << key;
    QQmlProperty::write(root, "opacity", CHESS_MAX_OPACITY);
    if( key==CH_ESCAPE_CODE )
    {
        click_mode = CH_NO_CLICK;
        hideUI();
    }
    else if( key==CH_BACKSPACE_CODE )
    {
        if( key_buf.length()>0 )
        {
            key_buf.remove( key_buf.length()-1, 1 );
        }
    }
    else if( key==CH_F1_CODE )
    {
        meta_mode = 1; ///FIXME: MAKE THIS TOGGLE
    }
    else if( key>CH_KEY_MIN && key<CH_KEY_MAX )
    {
        key_buf += (char)key;
        QQmlProperty::write(root, "ch_timer", false);
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
}

void ChProcessorW::sendLeftKey()
{
    INPUT input;
    input.type=INPUT_MOUSE;
    input.mi.dx=0;
    input.mi.dy=0;
    input.mi.dwFlags=(MOUSEEVENTF_LEFTDOWN|MOUSEEVENTF_LEFTUP);
    input.mi.mouseData=0;
    input.mi.dwExtraInfo=NULL;
    input.mi.time=0;
    SendInput(1,&input,sizeof(INPUT));
}

void ChProcessorW::sendRightKey()
{
    INPUT input;
    input.type=INPUT_MOUSE;
    input.mi.dx=0;
    input.mi.dy=0;
    input.mi.dwFlags=(MOUSEEVENTF_RIGHTDOWN|MOUSEEVENTF_RIGHTUP);
    input.mi.mouseData=0;
    input.mi.dwExtraInfo=NULL;
    input.mi.time=0;
    SendInput(1,&input,sizeof(INPUT));
}

void ChProcessorW::sendMiddleKey()
{
    INPUT input;
    input.type=INPUT_MOUSE;
    input.mi.dx=0;
    input.mi.dy=0;
    input.mi.dwFlags=(MOUSEEVENTF_MIDDLEDOWN|MOUSEEVENTF_MIDDLEUP);
    input.mi.mouseData=0;
    input.mi.dwExtraInfo=NULL;
    input.mi.time=0;
    SendInput(1,&input,sizeof(INPUT));
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

    int r_x = (x - 1) * 27;
    int r_y = (y - 1) * 22;

    int x2 = pt.x + r_x;
    int y2 = pt.y + r_y;

    SetCursorPos(x2, y2);
}

void ChProcessorW::activateWindow()
{
    DWORD dwCurrentThread = GetCurrentThreadId();
    DWORD dwFGThread = GetWindowThreadProcessId(GetForegroundWindow(), NULL);
    AttachThreadInput(dwCurrentThread, dwFGThread, TRUE);

    //Actions
    SetForegroundWindow(hWnd);
    SetActiveWindow(hWnd);
//    SetWindowPos(hWnd,HWND_TOPMOST,0,0,0,0,SWP_NOMOVE | SWP_NOSIZE);

    AttachThreadInput(dwCurrentThread, dwFGThread, FALSE);
}
