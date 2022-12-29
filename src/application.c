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
#include <application.h>
#include <gmodule.h>

#define META "SmipsApplication"
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static const gchar* description = NULL;
static const gchar* summary = NULL;

static int _getopt (lua_State* L)
{
  size_t namesz;
  Application* self = luaL_checkudata (L, 1, META);
  const gchar* name = luaL_checklstring (L, 2, &namesz);
  const Option* opt = options_lookup (name, namesz);

  if (opt == NULL)
    lua_pushnil (L);
  else
  {
    switch (opt->type)
    {
      case G_OPTION_ARG_INT:
        lua_pushinteger (L, G_STRUCT_MEMBER (gint, self, opt->offset));
        break;
      case G_OPTION_ARG_INT64:
        lua_pushinteger (L, G_STRUCT_MEMBER (gint64, self, opt->offset));
        break;
      case G_OPTION_ARG_DOUBLE:
        lua_pushnumber (L, G_STRUCT_MEMBER (gdouble, self, opt->offset));
        break;
      case G_OPTION_ARG_STRING:
        G_GNUC_FALLTHROUGH;
      case G_OPTION_ARG_FILENAME:
        lua_pushstring (L, G_STRUCT_MEMBER (gchar*, self, opt->offset));
        break;
      case G_OPTION_ARG_STRING_ARRAY:
        G_GNUC_FALLTHROUGH;
      case G_OPTION_ARG_FILENAME_ARRAY:
        {
          gchar** ar = G_STRUCT_MEMBER (gchar**, self, opt->offset);
          guint i, len = g_strv_length (ar);

          lua_createtable (L, len, 0);

          for (i = 0; i < len; i++)
          {
            lua_pushstring (L, ar [i]);
            lua_rawseti (L, -2, i);
          }
        }
        break;
    }
  }
return 1;
}

static int _main (lua_State* L)
{
  Application* self = luaL_checkudata (L, 1, META);
  int i, top, argc = (top = lua_gettop (L)) - 2;
  GOptionContext* context = NULL;
  GError* tmperr = NULL;
  gchar **argv, **_argv = NULL;
  gchar* stat [32];

  luaL_checktype (L, 2, LUA_TFUNCTION);

  if (G_N_ELEMENTS (stat) >= argc)
    argv = stat;
  else
  {
    _argv = g_new (gchar*, argc);
    argv = _argv;
  }

  for (i = 0; i < argc; i++)
  {
    if ((argv [i] = (gchar*) lua_tostring (L, i + 3)) == NULL)
    {
      _g_free0 (_argv);

      const gchar* typename = luaL_typename (L, i + 3);
      const gchar* message = lua_pushfstring (L, "expected string, got %s", typename);
        luaL_argerror (L, i + 3, message);
    }
  }

  context = g_option_context_new ("files ...");

  GOptionEntry entries [] =
  {
    { "output", 'o', 0, G_OPTION_ARG_FILENAME, & self->output, "Place output in FILE", "FILE", },
    G_OPTION_ENTRY_NULL,
  };

  g_option_context_add_main_entries (context, entries, "en_US");
  g_option_context_set_description (context, description);
  g_option_context_set_help_enabled (context, TRUE);
  g_option_context_set_ignore_unknown_options (context, FALSE);
  g_option_context_set_strict_posix (context, FALSE);
  g_option_context_set_summary (context, summary);
  g_option_context_set_translation_domain (context, "en_US");
  g_option_context_parse (context, &argc, &argv, &tmperr);
  g_option_context_free (context);

  if (G_LIKELY (tmperr == NULL))
  {
      lua_pushvalue (L, 2);
    for (i = 0; i < argc; i++)
      lua_pushstring (L, argv [i]);
      _g_free0 (_argv);

      lua_call (L, argc, 0);
      return 0;
  }
  else
  {
    if (tmperr->domain != G_OPTION_ERROR)
      gerror (tmperr);
    else
    {
      if (tmperr->code == G_OPTION_ERROR_FAILED)
        gerror (tmperr);
      else
      {
#if LUA_VERSION_NUM >= 502
        lua_pushinteger (L, LUA_RIDX_GLOBALS);
#else // LUA_VERSION_NUM < 502
        lua_pushvalue (L, LUA_GLOBALSINDEX);
#endif // LUA_VERSION_NUM
        lua_getfield (L, -1, "require");
        lua_pushliteral (L, "log");
        lua_call (L, 1, 1);
        lua_getfield (L, -1, "error");
        lua_pushstring (L, tmperr->message);
        lua_call (L, 1, 0);
        return 0;
      }
    }
  }
return 0;
}

G_MODULE_EXPORT
int luaopen_application (lua_State* L)
{
  lua_newuserdata (L, sizeof (Application));
  luaL_newmetatable (L, META);
  lua_createtable (L, 0, 0);
  lua_pushcfunction (L, _getopt);
  lua_setfield (L, -2, "getopt");
  lua_pushcfunction (L, _main);
  lua_setfield (L, -2, "main");
  lua_setfield (L, -2, "__index");
  lua_setmetatable (L, -2);
return 1;
}
