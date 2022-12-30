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

typedef struct _Unit Unit;
#define META "SmipsUnit"

struct _Unit
{
  int dummy;
};

static int new (lua_State* L)
{
  Unit* unit = NULL;

  unit = lua_newuserdata (L, sizeof (Unit));
#if LUA_VERSION_NUM >= 502
  luaL_setmetatable (L, META);
#else // LUA_VERSION_NUM < 502
  lua_getfield (L, LUA_REGISTRYINDEX, META);
  lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM
return 1;
}

static int __gc (lua_State* L)
{
return 0;
}

static int add_rinst (lua_State* L)
{
return 0;
}

static int add_iinst (lua_State* L)
{
return 0;
}

static int add_jinst (lua_State* L)
{
return 0;
}

static int add_tag (lua_State* L)
{
return 0;
}

G_MODULE_EXPORT
int luaopen_unit (lua_State* L)
{
  lua_createtable (L, 0, 5);

  lua_pushcfunction (L, new);
  lua_setfield (L, -2, "new");

  luaL_newmetatable (L, META);
  lua_pushcfunction (L, __gc);
  lua_setfield (L, -2, "__gc");
  lua_pushvalue (L, -2);
  lua_setfield (L, -2, "__index");
  lua_pop (L, 1);

  lua_pushcfunction (L, add_rinst);
  lua_setfield (L, -2, "add_rinst");
  lua_pushcfunction (L, add_iinst);
  lua_setfield (L, -2, "add_iinst");
  lua_pushcfunction (L, add_jinst);
  lua_setfield (L, -2, "add_jinst");
  lua_pushcfunction (L, add_tag);
  lua_setfield (L, -2, "add_tag");
return 1;
}
