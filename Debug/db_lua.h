#ifndef DBLUA_H
#define DBLUA_H

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

class DbLua
{
public:
    explicit DbLua();
    void run(QString word);
    ~DbLua();

private:
    void connectPipe();
    void sendKey(QString type, int keycode);
    void sendDebug(QString word);
    void sendPipe(const char *data);

    HANDLE hPipe;
    lua_State *lst;
};

#endif // DBLUA_H
