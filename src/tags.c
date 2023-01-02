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
#include <tag.h>
#include <tags.h>

#define META "SmipsTag"
#define checktag(L,idx) \
  (G_GNUC_EXTENSION ({ \
      lua_State* __L = ((L)); \
      int __idx = ((idx)); \
      void* __ud = NULL; \
      __ud = luaL_checkudata (__L, __idx, META); \
      *(SmipsTag**) __ud; \
    }))

static int __gc (lua_State* L)
{
  SmipsTag* self = checktag (L, 1);
  _smips_tag_unref (self);
return 0;
}

#if LUA_VERSION_NUM < 502
# define luaL_setmetatable(L, name) \
  G_STMT_START { \
    lua_getfield (L, LUA_REGISTRYINDEX, name); \
    lua_setmetatable (L, -2); \
  } G_STMT_END
#endif // LUA_VERSION_NUM

#define defnew(name,_type) \
  static int name (lua_State* L) \
  { \
    const gsize sz = sizeof (SmipsTag*); \
    const guint value = luaL_optinteger (L, 1, 0); \
    SmipsTag* self = _smips_tag_new_value ((_type), value); \
    SmipsTag** ref = lua_newuserdata (L, sz); \
    luaL_setmetatable (L, META); \
; \
      *ref = self; \
  return 1; \
  }
#if LUA_VERSION_NUM < 502
# undef luaL_setmetatable
#endif // LUA_VERSION_NUM

defnew (_abs, 0);
defnew (_rel, TAG_VALUE);
#undef defnew

static int __sum (lua_State* L)
{
  SmipsTag* tag1 = NULL;
  SmipsTag* tag2 = NULL;
  SmipsTag** ref = NULL;
  gsize sz = sizeof (SmipsTag*);
  guint invert = 0;

  lua_settop (L, 2);

  if (luaL_testudata (L, 2, META))
  {
    lua_insert (L, 1);
    invert = 1;
  }
  if (!luaL_testudata (L, 2, META))
  {
    lua_pushcfunction (L, _abs);
    lua_insert (L, 2);
    lua_call (L, 1, 1);
  }

  tag1 = checktag (L, 1 + invert);
  tag2 = checktag (L, 2 - invert);
  ref = lua_newuserdata (L, sz);
  *ref = _smips_tag_new_op (TAG_SUM, tag1, tag2);
#if LUA_VERSION_NUM >= 502
  luaL_setmetatable (L, META);
#else // LUA_VERSION_NUM < 502
  lua_getfield (L, LUA_REGISTRYINDEX, META);
  lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM
return 1;
}

G_MODULE_EXPORT
int luaopen_tags (lua_State* L)
{
  lua_createtable (L, 0, 1);
  luaL_newmetatable (L, META);
#ifdef LUA_ISJIT
  lua_pushliteral(L, META);
  lua_setfield(L, -2, "__name");
#endif // LUA_ISJIT
  lua_pushcfunction (L, __sum);
  lua_setfield (L, -2, "__sum");
  lua_pushcfunction (L, __gc);
  lua_setfield (L, -2, "__gc");
  lua_pushvalue (L, -2);
  lua_setfield (L, -2, "__index");
  lua_pop (L, 1);

  lua_pushcfunction (L, _abs);
  lua_setfield (L, -2, "abs");
  lua_pushcfunction (L, _rel);
  lua_setfield (L, -2, "rel");
return 1;
}
