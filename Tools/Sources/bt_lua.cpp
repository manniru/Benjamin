#include "bt_lua.h"
#include <QtDebug>
#include <QDir>

BtLua::BtLua(QObject *parent) : QObject(parent)
{
    lst = luaL_newstate();
    luaL_openlibs(lst);

    if( WaitNamedPipeA(BT_PIPE_ADDRESS, NMPWAIT_USE_DEFAULT_WAIT) )
    {
        hPipe = CreateFileA(BT_PIPE_ADDRESS,
                           GENERIC_WRITE,                   // dwDesiredAccess
                           0,                               // dwShareMode
                           nullptr,                         // lpSecurityAttributes
                           OPEN_EXISTING,                   // dwCreationDisposition
                           0,                               // dwFlagsAndAttributes
                           nullptr);                        // hTemplateFile

        if( hPipe==INVALID_HANDLE_VALUE )
        {
            qDebug() << "Error 120: Cannot create " BT_PIPE_ADDRESS;
        }
    }
    else
    {
        qDebug() << "Error 121: First create pipe " BT_PIPE_ADDRESS;
    }
}

void BtLua::run(QString word)
{ ///FIXME: solve directory problem
    QString current_dir = QDir::currentPath();
    QDir::setCurrent(KAL_SI_DIR);

    luaL_loadfile(lst, "main_w.lua");

    lua_pushstring(lst, word.toStdString().c_str());
    lua_setglobal(lst, "in_word");
    lua_pcall(lst, 0, LUA_MULTRET, 0);

    lua_getglobal(lst, "output");
    int output = lua_tonumber(lst, -1);

    lua_getglobal(lst, "k_type");
    QString ktype = lua_tostring(lst, -1);

    qDebug() << "LUA : " << output << ktype;

    sendPipe(ktype, output);

    QDir::setCurrent(current_dir);
}

BtLua::~BtLua()
{
    lua_close(lst);
}

void BtLua::sendPipe(QString type, int keycode)
{
    QString line(type + BT_PN_SEPARATOR + QString::number(keycode) + "\n");
    if( hPipe!=INVALID_HANDLE_VALUE )
    {
        DWORD dwWritten;
        if( !WriteFile(hPipe, line.toStdString().c_str(), (DWORD)line.length(), &dwWritten, nullptr) )
        {
            qDebug() << "Error: NamedPipe writing failed," << GetLastError();
        }

        if( dwWritten!=(DWORD)line.length() )
        {
            qDebug() << "Error: Wrong writing length. Total send char:"
                     << dwWritten << ", Total char:" << line.length();
        }
    }
    else
    {
        qDebug() << "Error: Invalid handle pipe for " BT_PIPE_ADDRESS;
    }
}
