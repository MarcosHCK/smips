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
#include <log.h>
#include <option.h>
#include <options.h>

#define META "SmipsOptions"
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))

static const gchar* description = NULL;
static const gchar* summary = NULL;

static int _getopt (lua_State* L)
{
  size_t namesz;
  const SmipsOptions* self = luaL_checkudata (L, 1, META);
  const gchar* name = luaL_checklstring (L, 2, &namesz);
  const SmipsOption* opt = _smips_options_lookup (name, namesz);
  const gchar* string = NULL;

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
        {
          if ((string = G_STRUCT_MEMBER (gchar*, self, opt->offset)) == NULL)
            lua_pushnil (L);
          else
            lua_pushstring (L, string);
        }
        break;
      case G_OPTION_ARG_STRING_ARRAY:
        G_GNUC_FALLTHROUGH;
      case G_OPTION_ARG_FILENAME_ARRAY:
        {
          gchar** ar = G_STRUCT_MEMBER (gchar**, self, opt->offset);
          guint i, len = g_strv_length (ar);

          if (ar == NULL)
            lua_pushnil (L);
          else
          {
            lua_createtable (L, len, 0);

            for (i = 0; i < len; i++)
            {
              lua_pushstring (L, ar [i]);
              lua_rawseti (L, -2, i);
            }
          }
        }
        break;
    }
  }
return 1;
}

static int _parse (lua_State* L)
{
  SmipsOptions* self = luaL_checkudata (L, 1, META);
  int i, top, argc = (top = lua_gettop (L)) - 1;
  gchar **argv, **_argv = NULL;
  GOptionContext* context = NULL;
  GError* tmperr = NULL;
  gchar* stat [32];

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
    }
  }

  self->output = NULL;

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
    for (i = 0; i < argc; i++)
      lua_pushstring (L, argv [i]);
      _g_free0 (_argv);
      return argc;
  }
  else
  {
    if (tmperr->domain != G_OPTION_ERROR)
      _smips_log_gerror (L, tmperr);
    else
    {
      if (tmperr->code == G_OPTION_ERROR_FAILED)
        _smips_log_gerror (L, tmperr);
      else
      {
        lua_pushstring (L, tmperr->message);
        g_error_free (tmperr);
        _smips_log_error (L, lua_tostring (L, -1));
      }
    }
  }
}

G_MODULE_EXPORT
int luaopen_options (lua_State* L)
{
  lua_newuserdata (L, sizeof (SmipsOptions));
  luaL_newmetatable (L, META);
  lua_createtable (L, 0, 0);
  lua_pushcfunction (L, _getopt);
  lua_setfield (L, -2, "getopt");
  lua_pushcfunction (L, _parse);
  lua_setfield (L, -2, "parse");
  lua_setfield (L, -2, "__index");
  lua_setmetatable (L, -2);
return 1;
}
