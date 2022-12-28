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

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

#ifndef LUA_PATHSEP
# ifdef LUA_PATH_SEP
#   define LUA_PATHSEP LUA_PATH_SEP
# else
#   error Fix this!
# endif // LUA_PATH_SEP
#endif // LUA_PATHSEP

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
