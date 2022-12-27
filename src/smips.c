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
#include <bundle.h>
#include <gio/gio.h>
#include <glib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

typedef struct _Reader Reader;
#define GRESROOT "/org/hck/smips"
#define BUFSZ (1024)

struct _Reader
{
  GInputStream* stream;
  gpointer buffer;
};

const char* preader (lua_State* L, void* ud, size_t* size)
{
  Reader* reader = (Reader*) ud;
  GInputStream* stream = reader->stream;
  gpointer buffer = reader->buffer;
  GError* tmperr = NULL;
  gsize read = 0;

  g_input_stream_read_all (stream, buffer, BUFSZ, &read, NULL, &tmperr);
  *size = (size_t) read;

  if (G_UNLIKELY (tmperr != NULL))
  {
    lua_pushfstring
    (L,
     "%s: %i: %s",
     g_quark_to_string
     (tmperr->domain),
      tmperr->code,
      tmperr->message);
    g_error_free (tmperr);
    lua_error (L);
  }
return (char*) buffer;
}

int pload (lua_State* L, const gchar* path)
{
  GResource* resource = NULL;
  GInputStream* stream = NULL;
  GError* tmperr = NULL;
  Reader reader = {0};
  int result;

  resource = bundle_get_resource ();
  stream = g_resource_open_stream (resource, path, 0, &tmperr);
    g_assert_no_error (tmperr);

  reader.stream = stream;
  reader.buffer = g_malloc (BUFSZ);

  result = lua_loadx (L, preader, &reader, "=smips", "bt");
  g_clear_pointer (&reader.stream, g_object_unref);
  g_clear_pointer (&reader.buffer, g_free);

  switch (result)
  {
    case LUA_ERRSYNTAX:
      luaL_error (L, "Internal error (syntax): %s", lua_tostring (L, -1));
      break;
    case LUA_ERRMEM:
      g_error ("Out of memory");
    default:
      g_assert_not_reached ();
      break;

    case LUA_OK:
      break;
  }
return 0;
}

int plink (lua_State* L)
{
return 0;
}

int report (lua_State* L)
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

int pmain (lua_State* L)
{
  const char* script = NULL;
  lua_Integer argc = lua_tointeger (L, 1);
  const char** argv = lua_touserdata (L, 2);
  lua_Integer i;

  lua_gc (L, LUA_GCSTOP, 0);
  luaL_openlibs (L);
  lua_gc (L, LUA_GCRESTART, -1);
  lua_settop (L, 0);

  lua_pushcfunction (L, plink);
  lua_getglobal (L, "package");
  lua_getfield (L, -1, "loaded");
  lua_remove (L, -2);
  lua_call (L, 1, 0);

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
