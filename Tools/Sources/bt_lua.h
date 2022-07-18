#ifndef BTLUA_H
#define BTLUA_H

#include <QObject>
extern "C" {
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <windows.h>
#include "config.h"

#define BT_PN_SEPARATOR ","

class BtLua : public QObject
{
    Q_OBJECT
public:
    explicit BtLua(QObject *parent = nullptr);
    void run(QString word);
    ~BtLua();

private:
    void sendPipe(QString type, int keycode);

    HANDLE hPipe;
    lua_State *lst;
};

#endif // BTLUA_H
