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
#include <gio/gio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

struct _Reader
{
  GInputStream* stream;
  gpointer buffer;
};

typedef struct _Reader Reader;
const gsize BUFSZ = 1024;

const char* reader (lua_State* L, void* ud, size_t* size)
{
  Reader* reader = (Reader*) ud;
  GInputStream* stream = reader->stream;
  gpointer buffer = reader->buffer;
  GError* tmperr = NULL;
  gsize read = 0;

  g_input_stream_read_all (stream, buffer, BUFSZ, &read, NULL, &tmperr);

  if (G_LIKELY (tmperr == NULL))
    *size = (size_t) read;
  else
  {
    g_object_unref (stream);

    lua_pushfstring
    (L, G_STRLOC ": ",
     "%s: %i: %s",
      g_quark_to_string
      (tmperr->domain),
       tmperr->code,
       tmperr->message);
    g_error_free (tmperr);
    lua_error (L);
  }
return (char*) buffer;
}

int load (lua_State* L, const gchar* filename)
{
  GFile* file = NULL;
  GError* tmperr = NULL;
  Reader data = {0};
  gchar* chunkname = NULL;
  gchar* basename = NULL;
  int result;

  file = (gpointer) g_file_new_for_commandline_arg (filename);
  data.stream = (gpointer) g_file_read (file, NULL, &tmperr);
  data.buffer = (gpointer) lua_newuserdata (L, BUFSZ);
  g_object_unref (file);

  if (G_UNLIKELY (tmperr != NULL))
  {
    lua_pushfstring
    (L, G_STRLOC ": "
     "%s: %i: %s",
     g_quark_to_string
     (tmperr->domain),
      tmperr->code,
      tmperr->message);
    g_error_free (tmperr);
    lua_error (L);
  }
  else
  {
    basename = g_path_get_basename (filename);
    chunkname = g_strconcat ("=", basename, NULL);
    g_clear_pointer (& basename, g_free);

    result =
    lua_loadx (L, reader, &data, chunkname, "t");
    g_clear_pointer (& data.stream, g_object_unref);
    g_clear_pointer (& chunkname, g_free);

    switch (result)
    {
      case LUA_ERRSYNTAX:
        luaL_error (L, "Syntax error: %s", lua_tostring (L, -1));
        break;

      case LUA_OK: break;

      case LUA_ERRMEM:
        g_error ("Out of memory");
      default:
        g_assert_not_reached ();
        break;
    }
  }
return result;
}

int writer (lua_State* L, const void* p, size_t sz, void* ud)
{
  GOutputStream* stream = (GOutputStream*) ud;
  GError* tmperr = NULL;
  gsize wrote = 0;

  g_output_stream_write_all (stream, p, sz, &wrote, NULL, &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
  {
    g_object_unref (stream);

    lua_pushfstring
    (L, G_STRLOC ": ",
     "%s: %i: %s",
      g_quark_to_string
      (tmperr->domain),
       tmperr->code,
       tmperr->message);
    g_error_free (tmperr);
    lua_error (L);
  }
return 0;
}

int save (lua_State* L, const gchar* filename)
{
  GOutputStream* stream = NULL;
  GFile* file = NULL;
  GError* tmperr = NULL;
  int result;

  file = (gpointer) g_file_new_for_commandline_arg (filename);
  stream = (gpointer) g_file_replace (file, NULL, FALSE, 0, NULL, &tmperr);
  g_object_unref (file);

  if (G_UNLIKELY (tmperr != NULL))
  {
    lua_pushfstring
    (L, G_STRLOC ": "
     "%s: %i: %s",
     g_quark_to_string
     (tmperr->domain),
      tmperr->code,
      tmperr->message);
    g_error_free (tmperr);
    lua_error (L);
  }
  else
  {
    result =
    lua_dump (L, writer, (void*) stream);
    g_clear_object (& stream);

    switch (result)
    {
      case LUA_OK: break;

      case LUA_ERRMEM:
        g_error ("Out of memory");
      default:
        g_assert_not_reached ();
        break;
    }
  }
return result;
}

int main (int argc, char* argv [])
{
  GOptionContext* ctx = NULL;
  const gchar* description = NULL;
  const gchar* summary = NULL;
  const gchar* output = NULL;
  GError* tmperr = NULL;
  lua_State* L = NULL;

  const GOptionEntry entries[] =
  {
    { "output", 'o', G_OPTION_FLAG_NONE, G_OPTION_ARG_STRING, &output, "Output compiled code to FILE", "FILE", },
  };

  ctx =
  g_option_context_new ("file");
  g_option_context_add_main_entries (ctx, entries, "en_US");
  g_option_context_set_description (ctx, description);
  g_option_context_set_help_enabled (ctx, TRUE);
  g_option_context_set_ignore_unknown_options (ctx, FALSE);
  g_option_context_set_strict_posix (ctx, FALSE);
  g_option_context_set_summary (ctx, summary);
  g_option_context_set_translation_domain (ctx, "en_US");

  g_option_context_parse (ctx, &argc, &argv, &tmperr);
  g_option_context_free (ctx);

  if (G_UNLIKELY (tmperr != NULL))
  {
    g_error
    (G_STRLOC ": "
     "%s: %i: %s",
     g_quark_to_string
     (tmperr->domain),
     tmperr->code,
     tmperr->message);
    g_error_free (tmperr);
    g_assert_not_reached ();
  }

  if (G_UNLIKELY (argc > 2))
  {
    g_error ("Only one file at the time");
    g_assert_not_reached ();
  }

  if (G_UNLIKELY ((L = luaL_newstate ()) == NULL))
  {
    g_error (G_STRLOC ": luaL_newstate () failed!");
    g_assert_not_reached ();
  }

  load (L, argv [1]);
  save (L, output);
  lua_close (L);
return 0;
}
