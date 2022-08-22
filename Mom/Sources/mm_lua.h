#ifndef MM_LUA_H
#define MM_LUA_H

#include <QString>
#include "mm_config.h"

extern "C"
{
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>
}

class MmLua
{
public:
    explicit MmLua();
    void run();
    ~MmLua();

private:
    lua_State *lst;
};

#endif // MM_LUA_H
