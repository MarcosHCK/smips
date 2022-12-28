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

typedef struct _Context Context;
typedef struct _Group Group;
typedef struct _Option Option;

#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
#define _newuserdata(L,type,name) (_newuserdata ((L),sizeof(type),(name)))

#define TCONTEXT  "OptionContext"
#define TGROUP    "OptionGroup"

struct _Context
{
  GOptionContext* context;
  GSList* groups;
};

struct _Group
{
  GOptionGroup* group;
  GStringChunk* chunk;
  GHashTable* options;
};

struct _Option
{
  gchar* _long;

  union
  {
    gchar _short;
    gchar _shorts [2];
  };

  GOptionArg type;

  union
  {
    gboolean b_value;
    gint i_value;
    gint64 li_value;
    gchar* s_value;
    gchar** sa_value;
    gdouble d_value;
  };
};

static void* (_newuserdata) (lua_State* L, size_t sz, const char* name)
{
  void* data =
  lua_newuserdata (L, sz);
#if LUA_VERSION_NUM >= 502
  luaL_setmetatable (L, name);
#else // LUA_VERSION_NUM < 502
  lua_getfield (L, LUA_REGISTRYINDEX, name);
  lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM
return data;
}

static int group__call (lua_State* L)
{
  static GHashFunc hf = (GHashFunc) g_str_hash;
  static GEqualFunc ef = (GEqualFunc) g_str_equal;
  static GDestroyNotify dn = (void*) g_string_chunk_free;
  const gchar* name = NULL;
  const gchar* desc = NULL;
  const gchar* help = NULL;
  Group* group = NULL;

  name = luaL_optstring (L, 2, "");
  desc = luaL_optstring (L, 3, "");
  help = luaL_optstring (L, 4, "");

  group = _newuserdata (L, Group, TGROUP);
  group->chunk = g_string_chunk_new (64);
  group->options = g_hash_table_new (hf, ef);
  group->group = g_option_group_new (name, desc, help, group->chunk, dn);
return 1;
}

static int group__gc (lua_State* L)
{
  Group* group = luaL_checkudata (L, 1, TGROUP);
  g_option_group_unref (group->group);
  g_hash_table_remove_all (group->options);
  g_hash_table_unref (group->options);
return 0;
}

struct OptionArg { int __bla__; GOptionArg type; };
const struct OptionArg* optionarg_lookup (register const char* str, register size_t len);

static int group_add_entry (lua_State* L)
{
  Group* self = luaL_checkudata (L, 1, TGROUP);
  size_t longsz, shortsz, typesz;
  Option* option = NULL;

  GOptionEntry entries [2] = {G_OPTION_ENTRY_NULL};
  const gchar* long_name = luaL_optlstring (L, 2, NULL, &longsz);
  const gchar* short_name = luaL_optlstring (L, 3, NULL, &shortsz);
  const gchar* description = luaL_optstring (L, 4, NULL);
  const gchar* arg_description = luaL_optstring (L, 5, NULL);
  const gchar* type = luaL_optlstring (L, 6, "none", &typesz);
  const gchar* flags = luaL_optstring (L, 7, NULL);

  if ((short_name == NULL) && (long_name == short_name))
  {
    luaL_error (L, "Either long or short names (or both) must be specified");
    g_assert_not_reached ();
  }
  else
  if ((short_name != NULL) && (shortsz > 1))
  {
    luaL_error (L, "Short options are short (single character only)");
    g_assert_not_reached ();
  }

  if (type != NULL)
  {
    const struct OptionArg* ttype;
    if ((ttype = optionarg_lookup (type, typesz)) != NULL)
      entries->arg = ttype->type;
    else
    {
      luaL_error (L, "Unknown option type '%s'", type);
      g_assert_not_reached ();
    }
  }

    static const Option empty = {0};
    option = (gpointer) g_string_chunk_insert_len (self->chunk, (gchar*) & empty, sizeof (empty));
  if (long_name != NULL)
    option->_long = g_string_chunk_insert_len (self->chunk, long_name, longsz);
  if (short_name != NULL)
    option->_short = short_name [0];
    option->type = entries->arg;
  if (long_name != NULL)
    g_hash_table_insert (self->options, option->_long, option);
  if (short_name != NULL)
    g_hash_table_insert (self->options, option->_shorts, option);

  entries->long_name = option->_long;
  entries->short_name = option->_short;
  entries->flags = 0;
  entries->arg_data = & option->i_value;
  entries->description = description;
  entries->arg_description = arg_description;

  g_option_group_add_entries (self->group, entries);
return 0;
}

static int group_add_entries (lua_State* L)
{
  Group* self = luaL_checkudata (L, 1, TGROUP);
  int i, top = lua_gettop (L);
#if LUA_VERSION_NUM >= 502
  lua_Unsigned j, size;
#else // LUA_VERSION_NUM < 502
  size_t j, size;
#endif // LUA_VERSION_NUM
  

  for (i = 1; i < (top + 1); i++)
  {
    luaL_checktype (L, i, LUA_TTABLE);
    lua_pushcfunction (L, group_add_entry);
#if LUA_VERSION_NUM >= 502
    size = lua_rawlen (L, -1);
#else // LUA_VERSION_NUM < 502
    size = lua_objlen (L, -1);
#endif // LUA_VERSION_NUM

    for (j = 1; j < (size + 1); j++)
    {
      lua_pushinteger (L, j);
      lua_gettable (L, -2);
    }

    lua_call (L, size, 0);
  }
return 0;
}

static int group_option (lua_State* L)
{
  Group* self = luaL_checkudata (L, 1, TGROUP);
  const gchar* name = luaL_checkstring (L, 2);
  Option* option = NULL;

  if (!g_hash_table_lookup_extended (self->options, name, NULL, (gpointer*) &option))
    lua_pushnil (L);
  else
  {
    switch (option->type)
    {
      case G_OPTION_ARG_NONE:
        lua_pushboolean (L, option->b_value);
        break;
      case G_OPTION_ARG_INT:
        G_GNUC_FALLTHROUGH;
      case G_OPTION_ARG_INT64:
        lua_pushinteger (L, option->li_value);
        break;
      case G_OPTION_ARG_DOUBLE:
        lua_pushnumber (L, option->d_value);
        break;
      case G_OPTION_ARG_STRING:
        G_GNUC_FALLTHROUGH;
      case G_OPTION_ARG_FILENAME:
        lua_pushstring (L, option->s_value);
        break;
      case G_OPTION_ARG_STRING_ARRAY:
        G_GNUC_FALLTHROUGH;
      case G_OPTION_ARG_FILENAME_ARRAY:
        {
          gchar** ar = option->sa_value;
          guint i, len = g_strv_length (ar);

          lua_createtable (L, len, 0);
          for (i = 0; i < len; i++)
          {
            lua_pushstring (L, ar [i]);
            lua_rawseti (L, -2, i + 1);
          }
        }
        break;

      default:
        g_assert_not_reached ();
    }
  }
return 1;
}

static int context__call (lua_State* L)
{
  Context* self = NULL;
  self = _newuserdata (L, Context, TCONTEXT);
  self->context = g_option_context_new (luaL_optstring (L, 2, NULL));
  self->groups = NULL;
return 1;
}

static int context__gc (lua_State* L)
{
  Context* self = luaL_checkudata (L, 1, TCONTEXT);
  g_option_context_free (self->context);
  g_slist_free (self->groups);
return 0;
}

static int context_add_group (lua_State* L)
{
  Context* self = luaL_checkudata (L, 1, TCONTEXT);
  Group* group = luaL_checkudata (L, 2, TGROUP);
  g_option_context_add_group (self->context, group->group);
return 0;
}

static int context_main_group (lua_State* L)
{
  Context* self = luaL_checkudata (L, 1, TCONTEXT);
  Group* group = luaL_checkudata (L, 2, TGROUP);
  g_option_context_set_main_group (self->context, group->group);
return 0;
}

static int context_parse (lua_State* L)
{
  Context* self = luaL_checkudata (L, 1, TCONTEXT);
  int i, top, argc = (top = lua_gettop (L)) - 1;
  gchar* stat [32] = {0};
  gchar** _argv = NULL;
  gchar** argv = NULL;
  GError* tmperr = NULL;

  if (G_N_ELEMENTS (stat) >= argc)
    argv = stat;
  else
  {
    _argv = g_new (gchar*, argc);
    argv = _argv;
  }

  for (i = 0; i < argc; i++)
  {
    if ((argv [i] = (gchar*) lua_tostring (L, i + 2)) == NULL)
    {
      _g_free0 (_argv);
      const gchar* typename = luaL_typename (L, i + 2);
      const gchar* message = lua_pushfstring (L, "expected string, got %s", typename);
      luaL_argerror (L, i + 2, message);
      g_assert_not_reached ();
    }
  }

  g_option_context_parse (self->context, &argc, &argv, &tmperr);

  if (G_LIKELY (tmperr == NULL))
  {
    for (i = 0; i < argc; i++)
      lua_pushstring (L, argv [i]);
      _g_free0 (_argv);
      return argc;
  }
  else
  {
    _g_free0 (_argv);

    if (g_error_matches (tmperr, G_OPTION_ERROR, G_OPTION_ERROR_FAILED))
      gerror (tmperr);
    else
    {
      lua_pushnil (L);
      lua_pushstring (L, tmperr->message);
      g_error_free (tmperr);
      return 2;
    }
  }
}

G_MODULE_EXPORT
int luaopen_options (lua_State* L)
{
  lua_createtable (L, 0, 0);

  /* Group */

  lua_createtable (L, 0, 0);

  luaL_newmetatable (L, TGROUP);
  lua_pushcfunction (L, group__gc);
  lua_setfield (L, -2, "__gc");
  lua_pushvalue (L, -2);
  lua_setfield (L, -2, "__index");
  lua_pop (L, 1);

  lua_createtable (L, 0, 1);
  lua_pushcfunction (L, group__call);
  lua_setfield (L, -2, "__call");
  lua_setmetatable (L, -2);

  lua_pushcfunction (L, group_add_entry);
  lua_setfield (L, -2, "add_entry");
  lua_pushcfunction (L, group_add_entries);
  lua_setfield (L, -2, "add_entries");
  lua_pushcfunction (L, group_option);
  lua_setfield (L, -2, "option");

  lua_setfield (L, -2, "Group");

  /* Context */

  lua_createtable (L, 0, 0);

  luaL_newmetatable (L, TCONTEXT);
  lua_pushcfunction (L, context__gc);
  lua_setfield (L, -2, "__gc");
  lua_pushvalue (L, -2);
  lua_setfield (L, -2, "__index");
  lua_pop (L, 1);

  lua_createtable (L, 0, 1);
  lua_pushcfunction (L, context__call);
  lua_setfield (L, -2, "__call");
  lua_setmetatable (L, -2);

  lua_pushcfunction (L, context_add_group);
  lua_setfield (L, -2, "add_group");
  lua_pushcfunction (L, context_main_group);
  lua_setfield (L, -2, "main_group");
  lua_pushcfunction (L, context_parse);
  lua_setfield (L, -2, "parse");

  lua_setfield (L, -2, "Context");
return 1;
}
