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
#include <smips.h>

#ifndef LUA_PATHSEP
# ifdef LUA_PATH_SEP
#   define LUA_PATHSEP LUA_PATH_SEP
# else
#   error Fix this!
# endif // LUA_PATH_SEP
#endif // LUA_PATHSEP

static const char* nexttemplate (lua_State* L, const char* path)
{
  const char* l;

  while (*path == *LUA_PATHSEP)
    path++;

  if (*path == '\0')
    return NULL;

  l = strchr (path, *LUA_PATHSEP);
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
            LUA_PATHSEP  GRESROOT "/" LUA_PATH_MARK ".luc";
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
        gerror (tmperr);
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

int rloader (lua_State* L)
{
  const char* path = NULL;
  const char* name = NULL;

  name = luaL_checkstring (L, 1);
  path = search (L, name);

  if (path != NULL)
  {
    pload (L, path);
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

int bloader (lua_State* L)
{
  const char* modname = NULL;
  const char* symname = NULL;
  GModule* module = NULL;
  gpointer sym = NULL;

  modname = luaL_checkstring (L, 1);
  modname = luaL_gsub (L, modname, ".", "_");
  modname = ignore (L, modname, LUA_IGMARK);
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
