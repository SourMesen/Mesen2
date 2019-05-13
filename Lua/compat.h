#ifndef COMPAT_H
#define COMPAT_H

#include "lua.h"
#include "lauxlib.h"

#ifndef _WIN32
	#define gai_strerrorA gai_strerror
#endif

#if LUA_VERSION_NUM==501
void luaL_setfuncs (lua_State *L, const luaL_Reg *l, int nup);
void *luaL_testudata ( lua_State *L, int arg, const char *tname);
#endif

#endif
