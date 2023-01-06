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
  const gchar* message = luaL_checkstring (L, 1);
  const int user = luaL_optinteger (L, 2, 1);
  const int level = luaL_optinteger (L, 3, 1);
    lua_settop (L, 1);
return _smips_log_error (L, level, user);
}

int _smips_log_error (lua_State* L, int level, int user)
{
  if (lua_gettop (L) < 1)
  lua_pushstring (L, "Failed!");
  lua_createtable (L, 0, 3);
#if LUA_VERSION_NUM >= 502
  luaL_setmetatable (L, META);
#else // LUA_VERSION_NUM < 502
  lua_getfield (L, LUA_REGISTRYINDEX, META);
  lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM
  lua_pushinteger (L, user);
  lua_setfield (L, -2, "user");
  lua_pushinteger (L, level);
  lua_setfield (L, -2, "level");
  lua_pushvalue (L, -2);
  lua_setfield (L, -2, "message");
  lua_error (L);
  g_assert_not_reached ();
}

int _smips_log_lerror (lua_State* L, int user, const gchar* message)
{
  lua_pushstring (L, message);
  _smips_log_error (L, 1, user);
}

int _smips_log_gerror (lua_State* L, int user, GError* error)
{
  lua_pushfstring
  (L,
   "%s: %i: %s",
   g_quark_to_string
   (error->domain),
    error->code,
    error->message);

  g_error_free (error);
  _smips_log_error (L, 1, user);
}

static int islogerror (lua_State* L, int idx)
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

static const char* trymessage (lua_State* L, int idx)
{
  const char* message;
  const char* typename;

  if ((message = lua_tostring (L, idx)) == NULL)
  {
    if (luaL_callmeta (L, idx, "__tostring") && lua_type (L, -1) == LUA_TSTRING)
      message = lua_tostring (L, -1);
    else
    {
      typename = lua_typename (L, lua_type (L, idx));
      message = lua_pushfstring (L, "(error object is a %s value)", typename);
    }
  }
return message;
}

int _smips_msgh (lua_State* L)
{
  const char* message;
  int level = 1;
  int user = 0;

  if (!islogerror (L, 1))
    message = trymessage (L, 1);
  else
  {
    lua_getfield (L, 1, "message");
    lua_getfield (L, 1, "level");
    lua_getfield (L, 1, "user");

    user = luaL_optinteger (L, -1, user);
    level = luaL_optinteger (L, -2, level);
    message = trymessage (L, -3);
  }

  if (user == 1)
    lua_pushstring (L, message);
  else
  {
    lua_pushfstring (L, "Internal error (run): %s", message);
    luaL_traceback (L, L, lua_tostring (L, -1), level);
  }
return 1;
}

G_MODULE_EXPORT
int luaopen_log (lua_State* L)
{
  luaL_newmetatable (L, META);
#if LUA_VERSION_NUM < 503
  lua_pushliteral (L, META);
  lua_setfield (L, -2, "__name");
#endif // LUA_VERSION_NUM
  lua_pop (L, 1);

  lua_createtable (L, 0, 1);
  lua_pushcfunction (L, _error);
  lua_setfield (L, -2, "error");
return 1;
}
