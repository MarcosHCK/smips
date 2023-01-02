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

static SmipsTag* _new (lua_State* L);
static int _abs (lua_State* L);
static int _rel (lua_State* L);

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

#define META "SmipsTag"
#define checktag(L,idx) \
  (G_GNUC_EXTENSION ({ \
      lua_State* __L = ((L)); \
      int __idx = ((idx)); \
      void* __ud = NULL; \
      __ud = luaL_checkudata (__L, __idx, META); \
      *(SmipsTag**) __ud; \
    }))

#define operation(name,_type) \
  static int name (lua_State* L) \
  { \
    SmipsTag* tag1 = NULL; \
    SmipsTag* tag2 = NULL; \
    SmipsTag* self = NULL; \
    guint invert = 0; \
; \
    lua_settop (L, 2); \
; \
    if (luaL_testudata (L, 2, META)) \
    { \
      lua_insert (L, 1); \
      invert = 1; \
    } \
    if (!luaL_testudata (L, 2, META)) \
    { \
      lua_pushcfunction (L, _abs); \
      lua_insert (L, 2); \
      lua_call (L, 1, 1); \
    } \
; \
    tag1 = checktag (L, 1 + invert); \
    tag2 = checktag (L, 2 - invert); \
; \
    self = _new (L); \
    self->type = TAG_OPER | (_type); \
    self->left = _smips_tag_ref (tag1); \
    self->right = _smips_tag_ref (tag2); \
  return 1; \
  }

  static int __unm (lua_State* L)
  {
    SmipsTag* tag1 = NULL;
    SmipsTag* self = NULL;

    tag1 = checktag (L, 1);

    self = _new (L);
    self->type = TAG_UNM;
    self->left = _smips_tag_ref (tag1);
    self->right = NULL;
  return 1;
  }

operation (__add, TAG_ADD)
operation (__sub, TAG_SUB)
operation (__mul, TAG_MUL)
operation (__div, TAG_DIV)
operation (__idiv, TAG_IDIV)
operation (__mod, TAG_MOD)
#undef operation

static int __gc (lua_State* L)
{
  SmipsTag* self = checktag (L, 1);
  _smips_tag_unref (self);
return 0;
}

static int __index (lua_State* L)
{
  size_t keysz;
  const SmipsTagIndex* index = NULL;
  const SmipsTag* self = checktag (L, 1);
  const gchar* key = luaL_checklstring (L, 2, &keysz);
  const gchar* value = NULL;

  if ((index = _smips_tag_index_lookup (key, keysz)) == NULL)
    lua_getfield (L, lua_upvalueindex (1), key);
  else
  {
    if ((self->type & index->type) == 0)
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

static SmipsTag* _new (lua_State* L)
{
  const int type = 0;
  const gsize sz = sizeof (SmipsTag*);
  SmipsTag* self = _smips_tag_new ();
  SmipsTag** ref = lua_newuserdata (L, sz);
#if LUA_VERSION_NUM >= 502
  luaL_setmetatable (L, META);
#else // LUA_VERSION_NUM
  lua_getfield (L, LUA_REGISTRYINDEX, META);
  lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM
    *ref = self;
return self;
}

static int _abs (lua_State* L)
{
  const int type = 0;
  const guint value = luaL_optinteger (L, 1, 0);
  SmipsTag* self = _new (L);
    self->type = TAG_VALUE | type;
    self->value = value;
return 1;
}

static int _rel (lua_State* L)
{
  const int type = TAG_REL;
  const guint value = luaL_optinteger (L, 1, 0);
  SmipsTag* self = _new (L);
    self->type = TAG_VALUE | type;
    self->value = value;
return 1;
}

void _smips_tag_print (SmipsTag* tag, int level)
{
  gchar stat [32];
  gchar* _idents = NULL;
  gchar* idents = NULL;

  static const gchar* ops [] =
  {
    "add", "sub",
    "mul", "div",
    "idiv", "mod",
    "unm",
  };

  if (level >= G_N_ELEMENTS (stat))
  {
    _idents = g_strnfill (level, ' ');
    idents = _idents;
  }
  else
  {
    int i;
    for (i = 0; i < level; i++)
      stat [i] = ' ';
      stat [level] = '\0';
      idents = stat;
  }

  if ((tag->type & TAG_VALUE) != 0)
  {
    const int idx = tag->type & TAG_REL;
    const gchar* value = (idx) ? "rel" : "abs";
    g_print ("%s- %s : %i\r\n", idents, value, tag->value);
    _g_free0 (_idents);
  }
  else
  {
    const int idx = (tag->type & TAG_OP_MASK);
    const gchar* value = ops [(idx >> 1) - 1];
    g_print ("%s- op : %s %i\r\n", idents, value, (idx >> 1) - 1);
    _g_free0 (_idents);

    if (tag->left != NULL)
      _smips_tag_print (tag->left, level + 1);
    if (tag->right != NULL)
      _smips_tag_print (tag->right, level + 1);
  }
}

static int print (lua_State* L)
{
  SmipsTag* self = checktag (L, 1);
  const int level = luaL_optinteger (L, 2, 0);
  _smips_tag_print (self, level);
return 0;
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
  lua_pushcfunction (L, __add);
  lua_setfield (L, -2, "__add");
  lua_pushcfunction (L, __sub);
  lua_setfield (L, -2, "__sub");
  lua_pushcfunction (L, __mul);
  lua_setfield (L, -2, "__mul");
  lua_pushcfunction (L, __div);
  lua_setfield (L, -2, "__div");
  lua_pushcfunction (L, __idiv);
  lua_setfield (L, -2, "__idiv");
  lua_pushcfunction (L, __mod);
  lua_setfield (L, -2, "__mod");
  lua_pushcfunction (L, __unm);
  lua_setfield (L, -2, "__unm");
  lua_pushcfunction (L, __gc);
  lua_setfield (L, -2, "__gc");
  lua_pushvalue (L, -2);
  lua_pushcclosure (L, __index, 1);
  lua_setfield (L, -2, "__index");
  lua_pop (L, 1);

  lua_pushcfunction (L, _abs);
  lua_setfield (L, -2, "abs");
  lua_pushcfunction (L, _rel);
  lua_setfield (L, -2, "rel");
  lua_pushcfunction (L, print);
  lua_setfield (L, -2, "print");
return 1;
}
