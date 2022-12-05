#ifndef AB_LUA_H
#define AB_LUA_H

#include <QString>
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <windows.h>
#include "config.h"

#define BT_PN_SEPARATOR ","

class AbLua
{
public:
    explicit AbLua();
    void run(QString word);
    ~AbLua();

private:
    void connectPipe();
    void sendKey(QString type, int keycode);
    void sendDebug(QString word);
    void sendPipe(const char *data);

    HANDLE hPipe;
    lua_State *lst;
};

#endif // AB_LUA_H
