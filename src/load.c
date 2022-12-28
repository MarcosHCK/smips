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

#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define _g_free0(var) ((var == NULL) ? NULL : (var = (g_free (var), NULL)))
typedef struct _Reader Reader;
#define BUFSZ (1024)

struct _Reader
{
  GInputStream* stream;
  gpointer buffer;
};

static const char* preader (lua_State* L, void* ud, size_t* size)
{
  Reader* reader = (Reader*) ud;
  GInputStream* stream = reader->stream;
  gpointer buffer = reader->buffer;
  GError* tmperr = NULL;
  gsize read = 0;

  g_input_stream_read_all (stream, buffer, BUFSZ, &read, NULL, &tmperr);
  *size = (size_t) read;

  if (G_UNLIKELY (tmperr != NULL))
    gerror (tmperr);
return (char*) buffer;
}

int pload (lua_State* L, const gchar* path)
{
  GResource* resource = NULL;
  GInputStream* stream = NULL;
  GError* tmperr = NULL;
  Reader reader = {0};
  gchar* basename = NULL;
  gchar* chunkname = NULL;
  int result;

  resource = bundle_get_resource ();
  stream = g_resource_open_stream (resource, path, 0, &tmperr);
    g_assert_no_error (tmperr);

  reader.stream = stream;
  reader.buffer = g_malloc (BUFSZ);

  basename = g_path_get_basename (path);
  chunkname = g_strconcat ("=", basename, NULL);
  result = lua_loadx (L, preader, &reader, chunkname, "bt");

  _g_object_unref0 (reader.stream);
  _g_free0 (reader.buffer);
  _g_free0 (chunkname);
  _g_free0 (basename);

  switch (result)
  {
    case LUA_ERRSYNTAX:
      luaL_error (L, "Internal error (syntax): %s", lua_tostring (L, -1));
      break;
    case LUA_ERRMEM:
      g_error ("Out of memory");
    default:
      g_assert_not_reached ();
      break;

    case LUA_OK:
      break;
  }
return 0;
}
