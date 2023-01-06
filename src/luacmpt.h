/* Copyright 2021-2025 MarcosHCK
 * This file is part of SMIPS Assembler.
 *
 * SMIPS Assembler is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * SMIPS Assembler is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with SMIPS Assembler. If not, see <http://www.gnu.org/licenses/>.
 *
 */
#ifndef __SMIPS_LUACMPT__
#define __SMIPS_LUACMPT__ 1
#include <lua.h>
#include <lauxlib.h>

#ifndef LUA_OK
# define LUA_OK (0)
#endif // LUA_OK
#ifndef LUA_PATH_SEP
# ifdef LUA_PATHSEP
#   define LUA_PATH_SEP LUA_PATHSEP
# else
#   define LUA_PATH_SEP ";"
# endif // LUA_PATH_SEP
#endif // LUA_PATHSEP
#ifndef LUA_PATH_MARK
# define LUA_PATH_MARK "?"
#endif // LUA_PATHSEP

#if __cplusplus
extern "C" {
#endif // __cplusplus

#if LUA_VERSION_NUM < 502
int repl_load (lua_State* L);
# ifndef LUA_ISJIT
void luaL_traceback (lua_State* L, lua_State* L1, const char* message, int level);
int luaL_testudata (lua_State *L, int idx, const char *tname);
# endif // LUA_ISJIT
#endif // LUA_VERSION_NUM

#if __cplusplus
}
#endif // __cplusplus

#endif // __SMIPS_LUACMPT__
