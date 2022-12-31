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
#include <gmodule.h>
#include <log.h>

#define META "SmipsLogError"

static int _error (lua_State* L)
{
  int top;
  if (1 >= (top = lua_gettop (L)))
  {
    luaL_checkstring (L, 1);
    lua_createtable (L, 0, 1);
    lua_pushvalue (L, -2);
    lua_setfield (L, -2, "message");
#if LUA_VERSION_NUM >= 502
    luaL_setmetatable (L, META);
#else // LUA_VERSION_NUM < 502
    lua_getfield (L, LUA_REGISTRYINDEX, META);
    lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM
    lua_error (L);
  }
  else
  {
#if LUA_VERSION_NUM >= 502
    lua_pushinteger (L, LUA_RIDX_GLOBALS);
    lua_gettable (L, LUA_REGISTRYINDEX);
#else // LUA_VERSION_NUM < 502
    lua_pushvalue (L, LUA_GLOBALSINDEX);
#endif // LUA_VERSION_NUM
    lua_getfield (L, -1, "string");
    lua_getfield (L, -1, "format");
    lua_insert (L, 1);
    lua_pop (L, 2);
    lua_pushcfunction (L, _error);
    lua_insert (L, 1);
    lua_call (L, top, 1);
    lua_call (L, 1, 0);
  }
return 0;
}

G_MODULE_EXPORT
int luaopen_log (lua_State* L)
{
  luaL_newmetatable (L, META);
  lua_pop (L, 1);

  lua_createtable (L, 0, 1);
  lua_pushcfunction (L, _error);
  lua_setfield (L, -2, "error");
return 1;
}

int _smips_log_error (lua_State* L, const gchar* error)
{
  lua_pushcfunction (L, _error);
  lua_pushstring (L, error);
    lua_call (L, 1, 0);
  g_assert_not_reached ();
}

int _smips_log_gerror (lua_State* L, GError* error)
{
  GQuark q = error->domain;
  gint c = error->code;
  const gchar* m = error->message;
  const gchar* d = g_quark_to_string (q);

  lua_pushfstring (L, "%s: %i: %s", d, c, m);
    g_error_free (error);
    lua_error (L);
  g_assert_not_reached ();
}

int _smips_islogerror (lua_State* L, int idx)
{
    int result = 0;
  if (lua_getmetatable (L, idx))
  {
#if LUA_VERSION_NUM >= 502
    luaL_getmetatable (L, META);
#else // LUA_VERSION_NUM < 502
    lua_getfield (L, LUA_REGISTRYINDEX, META);
#endif // LUA_VERSION_NUM
    if (lua_rawequal (L, -1, -2))
      result = 1;

    lua_pop (L, 2);
  }
return result;
}
