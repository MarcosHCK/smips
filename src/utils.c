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
#include <lua.h>
#include <lauxlib.h>
#include <luacmpt.h>

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static int pwd (lua_State* L)
{
  gchar* dir_ukwn = NULL;
  gsize dirsz = 0;

  dir_ukwn = g_get_current_dir ();
  dirsz = strlen (dir_ukwn);

  lua_pushlstring (L, dir_ukwn, dirsz);
  _g_free0 (dir_ukwn);
return 1;
}

static int build_path (lua_State* L)
{
  gchar* stat [32];
  gchar* path = NULL;
  gchar** _vector = NULL;
  gchar** vector = NULL;
  const gchar sep [] = { G_DIR_SEPARATOR, '\0' };
  int i, top = lua_gettop (L);

  if (G_N_ELEMENTS (vector) > top)
    vector = stat;
  else
  {
    _vector = g_new (gchar*, top + 1);
    vector = _vector;
  }

  for (i = 0; i < top; i++)
  {
    if ((vector [i] = (gchar*) lua_tostring (L, i + 1)) == NULL)
    {
      luaL_checktype (L, i + 1, LUA_TSTRING);
      g_assert_not_reached ();
    }
  }

  vector [top] = NULL;
  path = g_build_pathv (sep, vector);
  _g_free0 (_vector);
  lua_pushstring (L, path);
  _g_free0 (path);
return 1;
}

G_MODULE_EXPORT
int luaopen_utils (lua_State* L)
{
  lua_createtable (L, 0, 1);
  lua_pushcfunction (L, pwd);
  lua_setfield (L, -2, "pwd");
  lua_pushcfunction (L, build_path);
  lua_setfield (L, -2, "build_path");
return 1;
}
