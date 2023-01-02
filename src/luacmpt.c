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
#include <config.h>
#include <luacmpt.h>

#if LUA_VERSION_NUM < 502
# ifndef LUA_ISJIT

#define LEVELS1 12
#define LEVELS2 10

 /*
 * luaL_traceback campatibility patch based
 * upon 'db_errorfb' function in file
 * ldblib.c, Lua 5.1 source
 */
void luaL_traceback (lua_State* L, lua_State* L1, const char* message, int level)
{
  lua_Debug ar;
  int firstpart = 1;
  int top = lua_gettop (L);
  lua_pushstring (L, message ? message : "");
  lua_pushliteral (L, "\nstack traceback:");

  while (lua_getstack (L1, level++, &ar))
  {
    if (level > LEVELS1 && firstpart)
    {
      if (!lua_getstack (L1, level + LEVELS2, &ar))
        --level;
      else
      {
        lua_pushliteral (L, "\n\t...");
        while (lua_getstack (L1, level + LEVELS2, &ar))
          ++level;
      }

      firstpart = 0;
      continue;
    }

    lua_pushliteral (L, "\n\t");
    lua_getinfo (L1, "Snl", &ar);
    lua_pushfstring (L, "%s:", ar.short_src);

    if (ar.currentline > 0)
      lua_pushfstring (L, "%d:", ar.currentline);
    if (*ar.namewhat != '\0')
      lua_pushfstring (L, " in function " LUA_QS, ar.name);
    else
    {
      switch (*ar.what)
      {
        case 'm':
          lua_pushliteral (L, " in main chunk");
          break;
        case 'C':
        case 't':
          lua_pushliteral (L, " ?");
          break;
        default:
          lua_pushfstring (L, " in function <%s:%d>", ar.short_src, ar.linedefined);
          break;
      }
    }

    lua_concat (L, lua_gettop (L) - top);
  }

  lua_concat (L, lua_gettop (L) - top);
}

int luaL_testudata (lua_State *L, int idx, const char* tname)
{
  void* ud = lua_touserdata (L, idx);
  int itis = 0;

  if (ud != NULL)
  {
    if (lua_getmetatable (L, idx))
    {
      lua_getfield (L, LUA_REGISTRYINDEX, tname);
      if (lua_rawequal (L, -1, -2))
        itis = 1;

      lua_pop (L, 2);
    }
  }
return itis;
}

# endif // LUA_ISJIT
#endif // LUA_VERSION_NUM
