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
#include <smips.h>

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static int checkarg (lua_State* L)
{
  const char* type = NULL;
  const char* got = NULL;
  int top = lua_gettop (L);
  int i, argn, result = 0;

  argn = luaL_checkinteger (L, 1);
  got = luaL_typename (L, 2);

  luaL_Buffer B;
  luaL_buffinit (L, & B);
  luaL_addstring (& B, "expected");

  for (i = 3; i < top; i++)
  {
    type = luaL_checkstring (L, i);

    if (!g_strcmp0 (got, type))
    {
      result = TRUE;
      break;
    }
    else
    {
      if (i == 3)
      {
        lua_pushfstring (L, " %s", type);
        luaL_addvalue (& B);
      }
      else
      {
        lua_pushfstring (L, " o %s", type);
        luaL_addvalue (& B);
      }
    }
  }

  if (result)
  {
    luaL_pushresult (& B);
    lua_pop (L, 1);
  }
  else
  {
    lua_pushfstring (L, ", got %s", got);
    luaL_addvalue (& B);
    luaL_pushresult (& B);

    type = lua_tostring (L, -1);
    luaL_argerror (L, argn, type);
  }
return 0;
}

static int report (lua_State* L)
{
  const gchar* err = lua_tostring(L, 1);
  if (G_UNLIKELY (err == NULL))
  {
    if(luaL_callmeta (L, 1, "__tostring") && lua_type (L, -1) == LUA_TSTRING)
      err = lua_tostring (L, -1);
    else
      err = lua_pushfstring (L, "(error object is a %s value)", lua_typename (L, 1));
  }

  luaL_traceback (L, L, err, 1);
return 1;
}

static int pmain (lua_State* L)
{
  const char* script = NULL;
  lua_Integer argc = lua_tointeger (L, 1);
  const char** argv = lua_touserdata (L, 2);
#if LUA_VERSION_NUM >= 502
  lua_Unsigned size;
#else // LUA_VERSION_NUM < 502
  size_t size;
#endif // LUA_VERSION_NUM
  lua_Integer i;

  lua_gc (L, LUA_GCSTOP, 0);
  luaL_openlibs (L);
  lua_gc (L, LUA_GCRESTART, -1);
  lua_settop (L, 0);

  lua_getglobal (L, "package");
#if LUA_VERSION_NUM >= 502
  lua_pushliteral (L, "searchers");
#else // LUA_VERSION_NUM < 502
  lua_pushliteral (L, "loaders");
#endif // LUA_VERSION_NUM
  lua_gettable (L, -2);
#if LUA_VERSION_NUM >= 502
  size = lua_rawlen (L, -1);
#else // LUA_VERSION_NUM < 502
  size = lua_objlen (L, -1);
#endif // LUA_VERSION_NUM
  lua_pushinteger (L, size + 1);
  lua_pushcfunction (L, rloader);
  lua_settable (L, -3);
  lua_pushinteger (L, size + 2);
  lua_pushcfunction (L, bloader);
  lua_settable (L, -3);
  lua_pop (L, 2);

#if LUA_VERSION_NUM >= 502
  lua_pushinteger (L, LUA_RIDX_GLOBALS);
#else // LUA_VERSION_NUM < 502
  lua_pushvalue (L, LUA_GLOBALSINDEX);
#endif // LUA_VERSION_NUM
  lua_pushliteral (L, "argv0");
  lua_pushstring (L, argv [0]);
  lua_settable (L, -3);
  lua_pushliteral (L, "checkArg");
  lua_pushcfunction (L, checkarg);
  lua_settable (L, -3);
  lua_pop (L, 1);

#if LUA_VERSION_NUM < 502
  lua_getfield (L, LUA_GLOBALSINDEX, "table");
  lua_getfield (L, LUA_GLOBALSINDEX, "unpack");
  lua_setfield (L, -2, "unpack");
  lua_pop (L, 1);
  lua_pushnil (L);
  lua_setfield (L, LUA_GLOBALSINDEX, "unpack");
#endif // LUA_VERSION_NUM

  g_assert (lua_gettop (L) == 0);
  lua_pushcfunction (L, report);
  pload (L, GRESROOT "/smips.luc");

  for (i = 1; i < argc; i++)
    lua_pushstring (L, argv [i]);
  switch (lua_pcall (L, argc - 1, 1, 1))
  {
    case LUA_OK:
      break;
    case LUA_ERRRUN:
      luaL_error (L, "Internal error (run): %s", lua_tostring (L, -1));
      break;

    case LUA_ERRMEM:
      g_error ("Out of memory");
      g_assert_not_reached ();
      break;
    case LUA_ERRERR:
      g_error ("(" G_STRLOC ") Fix this!");
    default:
      g_assert_not_reached ();
      break;
  }
return 1;
}

int (gerror) (lua_State* L, const char* loc, GError* error)
{
  const GQuark q = error->domain;
  const guint c = error->code;
  const gchar* m = error->message;
  const gchar* d = g_quark_to_string (q);

  lua_pushfstring (L, "%s: %s: %i: %s", loc, d, c, m);
  g_error_free ((GError*) error);
  lua_error (L);
  for (;;);
}

int main (int argc, char* argv[])
{
#if G_PLATFORM_WIN32
  argv = g_win32_get_command_line ();
  argc = g_strv_length (argv);
#endif // G_PLATFORM_WIN32

  lua_State* L;
  int result;

  L = luaL_newstate ();

  if (G_UNLIKELY (L == NULL))
  {
    g_error ("luaL_newstate (): failed!");
    g_assert_not_reached ();
  }

  lua_pushcfunction (L, pmain);
  lua_pushinteger (L, argc);
  lua_pushlightuserdata (L, argv);

  switch (lua_pcall (L, 2, 1, 0))
  {
    case LUA_OK:
      result = lua_tointeger (L, -1);
        lua_pop (L, 1);
      goto out;
    case LUA_ERRRUN:
      g_warning (lua_tostring (L, -1));
        lua_pop (L, 1);
      goto out;

    case LUA_ERRMEM:
      g_error ("Out of memory");
      g_assert_not_reached ();
      break;
    case LUA_ERRERR:
      g_error ("(" G_STRLOC ") Fix this!");
    default:
      g_assert_not_reached ();
      break;

    out:
#if DEVELOPER == 1
      g_assert (lua_gettop (L) == 0);
#endif // DEVELOPER
      break;
  }

  lua_close (L);
#if G_PLATFORM_WIN32
  g_strfreev (argv);
#endif // G_PLATFORM_WIN32
return result;
}
