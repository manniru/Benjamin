#ifndef BT_LUA_H
#define BT_LUA_H

#include <QString>
extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}
#include <windows.h>
#include "config.h"
#include "bt_state.h"

#define BT_PN_SEPARATOR ","

class BtLua
{
public:
    explicit BtLua(BtState *st);
    void run(QString word);
    ~BtLua();

private:
    void connectPipe();
    void sendKey(QString type, int keycode);
    void sendDebug(QString word);
    void sendPipe(const char *data);

    HANDLE hPipe;
    lua_State *lst;
    BtState *state;
};

#endif // BT_LUA_H
