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

static int half2buf (lua_State* L)
{
  const int min = lua_upvalueindex (1);
  const int max = lua_upvalueindex (2);
  const int sz = 2;

  union __half2buf
  {
    guint16 whole;
    gchar buf [sz];
  } tag;

  G_STATIC_ASSERT (G_SIZEOF_MEMBER (union __half2buf, whole) == sz);
  G_STATIC_ASSERT (sizeof (*tag.buf) == 1);

#if LUA_VERSION_NUM >= 502
  if (lua_compare (L, 1, min, LUA_OPLT) || !lua_compare (L, 1, max, LUA_OPLE))
#else // LUA_VERSION_NUM < 502
  if (lua_lessthan (L, 1, min) || !(lua_lessthan (L, 1, max) || lua_equal (L, 1, max)))
#endif // LUA_VERSION_NUM
  luaL_argerror (L, 1, "too big or to small number");

  guint16 value = luaL_checkinteger (L, 1);
  guint16 corct = GUINT16_TO_LE (value);
    tag.whole = corct;
  lua_pushlstring (L, tag.buf, sz);
return 1;
}

static int word2buf (lua_State* L)
{
  const int min = lua_upvalueindex (1);
  const int max = lua_upvalueindex (2);
  const int sz = 4;

  union __word2buf
  {
    guint32 whole;
    gchar buf [sz];
  } tag;

  G_STATIC_ASSERT (G_SIZEOF_MEMBER (union __word2buf, whole) == sz);
  G_STATIC_ASSERT (sizeof (*tag.buf) == 1);

#if LUA_VERSION_NUM >= 502
  if (lua_compare (L, 1, min, LUA_OPLT) || !lua_compare (L, 1, max, LUA_OPLE))
#else // LUA_VERSION_NUM < 502
  if (lua_lessthan (L, 1, min) || !(lua_lessthan (L, 1, max) || lua_equal (L, 1, max)))
#endif // LUA_VERSION_NUM
  luaL_argerror (L, 1, "too big or to small number");

  guint32 value = luaL_checkinteger (L, 1);
  guint32 corct = GUINT32_TO_LE (value);
    tag.whole = corct;
  lua_pushlstring (L, tag.buf, sz);
return 1;
}

G_MODULE_EXPORT
int luaopen_utils (lua_State* L)
{
  lua_createtable (L, 0, 4);
  lua_pushcfunction (L, pwd);
  lua_setfield (L, -2, "pwd");
  lua_pushcfunction (L, build_path);
  lua_setfield (L, -2, "build_path");
  lua_pushinteger (L, 0);
  lua_pushinteger (L, G_MAXUINT16);
  lua_pushcclosure (L, half2buf, 2);
  lua_setfield (L, -2, "half2buf");
  lua_pushinteger (L, 0);
  lua_pushinteger (L, G_MAXUINT32);
  lua_pushcclosure (L, word2buf, 2);
  lua_setfield (L, -2, "word2buf");
return 1;
}
