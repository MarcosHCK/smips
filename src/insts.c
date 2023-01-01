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
#include <inst.h>
#include <insts.h>

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define META "SmipsInst"

static int __gc (lua_State* L)
{
  SmipsInst* self = luaL_checkudata (L, 1, META);
  _g_free0 (self->cs);
return 0;
}

static int __index (lua_State* L)
{
  size_t keysz;
  const gchar* value = NULL;
  const SmipsInstIndex* index = NULL;
  const SmipsInst* self = luaL_checkudata (L, 1, META);
  const gchar* key = luaL_checklstring (L, 2, &keysz);

  if ((index = _smips_inst_index_lookup (key, keysz)) == NULL)
    lua_getfield (L, lua_upvalueindex (1), key);
  else
  {
    if ((index->validin & self->type) == 0)
      lua_pushnil (L);
    else
    {
      switch (index->type)
      {
        case LUA_TNUMBER:
          lua_pushinteger (L, G_STRUCT_MEMBER (guint, self, index->offset));
          break;
        case LUA_TSTRING:
          {
            if ((value = G_STRUCT_MEMBER (gchar*, self, index->offset)) == NULL)
              lua_pushnil (L);
            else
              lua_pushstring (L, value);
          }
          break;
      }
    }
  }
return 1;
}

static int __newindex (lua_State* L)
{
  size_t keysz;
  const SmipsInstIndex* index = NULL;
  const SmipsInst* self = luaL_checkudata (L, 1, META);
  const gchar* key = luaL_checklstring (L, 2, &keysz);

  if ((index = _smips_inst_index_lookup (key, keysz)) == NULL)
    luaL_error (L, "Unknown field '%s'", key);
  else
  {
    if ((index->validin & self->type) == 0)
      luaL_error (L, "Unknown field '%s'", key);
    else
    {
      switch (index->type)
      {
        case LUA_TNUMBER:
          *((guint*) G_STRUCT_MEMBER_P (self, index->offset)) = (guint) luaL_checkinteger (L, 3);
          break;
        case LUA_TSTRING:
          {
            size_t valuesz;
            gchar* newvalue = (gchar*) luaL_checklstring (L, 3, &valuesz);
            gchar** value = (gchar**) G_STRUCT_MEMBER_P (self, index->offset);
              g_clear_pointer (value, g_free);
              *value = g_strndup (newvalue, (gsize) valuesz);
          }
          break;
      }
    }
  }
return 0;
}

static int new (lua_State* L)
{
  SmipsInst* self = NULL;
  guint opcode = 0;

  opcode = luaL_checkinteger (L, 1);
  self = lua_newuserdata (L, sizeof (SmipsInst));

#if LUA_VERSION_NUM >= 502
  luaL_setmetatable (L, META);
#else // LUA_VERSION_NUM < 502
  lua_getfield (L, LUA_REGISTRYINDEX, META);
  lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM
  memset (self, 0, sizeof (SmipsInst));
  self->opcode = opcode & 0x3f;
  self->type = R_INST | I_INST | J_INST; /* polymorphic */
return 1;
}

#define typex(c,n) \
  static int type##c (lua_State* L) \
  { \
    SmipsInst* self = luaL_checkudata (L, 1, META); \
    const guint type = ((n)); \
      if ((type & (~MASK_INST)) != 0) \
        luaL_error (L, "Unknown type %i", type); \
      if (self->type & type == 0) \
        luaL_error (L, "Can not assume type %i", type); \
        self->type = type; \
    lua_settop (L, 1); \
    return 1; \
  }

typex (r, R_INST)
typex (i, I_INST)
typex (j, J_INST)
#undef typex

G_MODULE_EXPORT
int luaopen_insts (lua_State* L)
{
  lua_createtable (L, 0, 1);

  luaL_newmetatable (L, META);
#ifdef LUA_ISJIT
  lua_pushliteral (L, META);
  lua_setfield (L, -2, "__name");
#endif // LUA_ISJIT
  lua_pushcfunction (L, __gc);
  lua_setfield (L, -2, "__gc");
  lua_pushvalue (L, -2);
  lua_pushcclosure (L, __index, 1);
  lua_setfield (L, -2, "__index");
  lua_pushcfunction (L, __newindex);
  lua_setfield (L, -2, "__newindex");
  lua_pop (L, 1);

  lua_pushcfunction (L, new);
  lua_setfield (L, -2, "new");
  lua_pushcfunction (L, typer);
  lua_setfield (L, -2, "typer");
  lua_pushcfunction (L, typei);
  lua_setfield (L, -2, "typei");
  lua_pushcfunction (L, typej);
  lua_setfield (L, -2, "typej");
return 1;
}
