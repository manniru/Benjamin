#include "mm_lua.h"
#include <QtDebug>
#include <QDir>

MmLua::MmLua()
{
    lst = luaL_newstate();
    luaL_openlibs(lst);
}

void MmLua::run()
{ ///FIXME: solve directory problem
    QString current_dir = QDir::currentPath();
    QDir::setCurrent(RE_FIREFOX_DIR);

    luaL_loadfile(lst, "init_db.lua");
    lua_pcall(lst, 0, LUA_MULTRET, 0);

    QDir::setCurrent(current_dir);
}

MmLua::~MmLua()
{
    lua_close(lst);
}
