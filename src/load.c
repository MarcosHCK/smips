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
#include <gio/gio.h>
#include <load.h>
#include <log.h>

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
typedef struct _Reader Reader;
#define BUFSZ (1024)

extern GResource* bundle_get_resource (void);

struct _Reader
{
  GInputStream* stream;
  gpointer buffer;
};

static const char* nexttemplate (lua_State* L, const char* path)
{
  const char* l;

  while (*path == *LUA_PATH_SEP)
    path++;

  if (*path == '\0')
    return NULL;

  l = strchr (path, *LUA_PATH_SEP);
  if (l == NULL)
    l = path + strlen(path);

  lua_pushlstring(L, path, l - path);
return l;
}

static const char* search (lua_State* L, const char* name)
{
  luaL_Buffer B;
  const char* remain = NULL;
  const char* fullpath = NULL;
  const char* template = NULL;
  GResource* resource = NULL;
  GError* tmperr = NULL;

  static const char* rpath = NULL;
  if (g_once_init_enter (&rpath))
  {
    const char* __path = GRESROOT "/" LUA_PATH_MARK ".lua"
           LUA_PATH_SEP  GRESROOT "/" LUA_PATH_MARK ".luc";
    g_once_init_leave (&rpath, __path);
  }

  remain = rpath;
  resource = bundle_get_resource ();
  name = luaL_gsub (L, name, ".", LUA_DIRSEP);

  luaL_buffinit (L, &B);

  while ((remain = nexttemplate (L, remain)) != NULL)
  {
    template = lua_tostring (L, -1);
    fullpath = luaL_gsub (L, template, LUA_PATH_MARK, name);

    lua_remove (L, -2);
    g_resource_get_info (resource, fullpath, 0, NULL, NULL, &tmperr);

    if (G_LIKELY (tmperr == NULL))
      return fullpath;
    else
    {
      if (!g_error_matches (tmperr, G_RESOURCE_ERROR, G_RESOURCE_ERROR_NOT_FOUND))
        _smips_log_gerror (L, 0, tmperr);
      else
      {
        g_clear_error (& tmperr);
        lua_pushfstring (L, "\n\tno resource '%s'", lua_tostring (L, -1));
        lua_remove (L, -2);
        luaL_addvalue (& B);
      }
    }
  }

  luaL_pushresult (& B);
return NULL;
}

static const char* ignore (lua_State* L, const char* name, const char* ig)
{
  const char* where;
  if ((where = g_strstr_len (name, -1, ig)) != NULL)
  {
    g_assert (where > name);
    lua_pushlstring (L, name, where - name);
    return lua_tostring (L, -1);
  }
return name;
}

static const char* _reader (lua_State* L, void* ud, size_t* size)
{
  Reader* reader = (Reader*) ud;
  GInputStream* stream = reader->stream;
  gpointer buffer = reader->buffer;
  GError* tmperr = NULL;
  gsize read = 0;

  g_input_stream_read_all (stream, buffer, BUFSZ, &read, NULL, &tmperr);
  *size = (size_t) read;

  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 0, tmperr);
return (char*) buffer;
}

int _smips_luc_loader (lua_State* L)
{
  const char* path = NULL;
  const char* name = NULL;

  name = luaL_checkstring (L, 1);
  path = search (L, name);

  if (path != NULL)
  {
    _smips_load (L, path);
  }
#if LUA_VERSION_NUM >= 502
  else
  {
    lua_pushnil (L);
    lua_insert (L, -2);
    return 2;
  }
#endif // LUA_VERSION_NUM
return 1;
}

int _smips_sym_loader (lua_State* L)
{
  const char* modname = NULL;
  const char* symname = NULL;
  GModule* module = NULL;
  gpointer sym = NULL;

  modname = luaL_checkstring (L, 1);
  modname = luaL_gsub (L, modname, ".", "_");
#if (LUA_VERSION_NUM < 502) || defined(LUA_ISJIT)
  modname = ignore (L, modname, LUA_IGMARK);
#endif // LUA_VERSION_NUM
  symname = lua_pushfstring (L, "luaopen_%s", modname);
  module = g_module_open (NULL, G_MODULE_BIND_LAZY);

  g_module_symbol (module, symname, &sym);
  g_module_close (module);

  if (sym != NULL)
    lua_pushcfunction (L, (lua_CFunction) sym);
  else
  {
#if LUA_VERSION_NUM >= 502
    lua_pushnil (L);
#endif // LUA_VERSION_NUM
    lua_pushfstring (L, "\n\tno symbol '%s'", symname);
#if LUA_VERSION_NUM >= 502
    return 2;
#endif // LUA_VERSION_NUM
  }
return 1;
}

int _smips_load (lua_State* L, const gchar* path)
{
  GResource* resource = NULL;
  GInputStream* stream = NULL;
  GError* tmperr = NULL;
  Reader reader = {0};
  gchar* basename = NULL;
  gchar* chunkname = NULL;
  int result;

  resource = bundle_get_resource ();
  stream = g_resource_open_stream (resource, path, 0, &tmperr);
    g_assert_no_error (tmperr);

  reader.stream = stream;
  reader.buffer = g_malloc (BUFSZ);

  basename = g_path_get_basename (path);
  chunkname = g_strconcat ("=", basename, NULL);
#if defined(LUA_ISJIT)
  result = lua_loadx (L, _reader, &reader, chunkname, "b");
#elif LUA_VERSION_NUM >= 502
  result = lua_load (L, _reader, &reader, chunkname, "b");
#else // LUA_VERSION_NUM < 502
  result = lua_load (L, _reader, &reader, chunkname);
#endif // LUA_VERSION_NUM

  _g_object_unref0 (reader.stream);
  _g_free0 (reader.buffer);
  _g_free0 (chunkname);
  _g_free0 (basename);

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
