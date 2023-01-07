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
#include <bank.h>
#include <gio/gio.h>
#include <gmodule.h>
#include <lua.h>
#include <lauxlib.h>
#include <luacmpt.h>
#include <log.h>

typedef struct _SmipsBank SmipsBank;
#define _g_object_unref0(var) ((var == NULL) ? NULL : (var = (g_object_unref (var), NULL)))
#define META "SmipsBank"

struct _SmipsBank
{
  union
  {
    gpointer object;
    GSeekable* seekable;
    GOutputStream* stream;
  };
};

static int __gc (lua_State* L)
{
  SmipsBank* self = luaL_checkudata (L, 1, META);
  g_clear_object (&self->object);
return 0;
}

static int _new (lua_State* L)
{
  GError* tmperr = NULL;
  GFile* file = NULL;
  GFileOutputStream* stream = NULL;
  const GType gtype = smips_raw2_stream_get_type ();
  const gsize sz = sizeof (SmipsBank);
  const gchar* name = luaL_checkstring (L, 1);
  SmipsBank* self = lua_newuserdata (L, sz);
#if LUA_VERSION_NUM >= 502
  luaL_setmetatable (L, META);
#else // LUA_VERSION_NUM < 502
  lua_getfield (L, LUA_REGISTRYINDEX, META);
  lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM

  file = g_file_new_for_commandline_arg (name);
  stream = g_file_replace (file, NULL, FALSE, 0, NULL, &tmperr);
  _g_object_unref0 (file);

  if (G_UNLIKELY (tmperr == NULL))
  {
    self->object = g_object_new (gtype, "base-stream", stream, NULL);
                  g_object_unref (stream);
  }
  else
  {
    if (tmperr->domain != G_IO_ERROR)
      _smips_log_gerror (L, 0, tmperr);
    else
    {
      switch (tmperr->code)
      {
        case G_IO_ERROR_IS_DIRECTORY:
        case G_IO_ERROR_NOT_REGULAR_FILE:
        case G_IO_ERROR_INVALID_FILENAME:
        case G_IO_ERROR_FILENAME_TOO_LONG:
          _smips_log_gerror (L, 1, tmperr);
          break;
        default:
          _smips_log_gerror (L, 0, tmperr);
          break;
      }
    }
  }
return 1;
}

static int _close (lua_State* L)
{
  SmipsBank* self = luaL_checkudata (L, 1, META);
  GError* tmperr = NULL;

  g_output_stream_close (self->stream, NULL, &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 0, tmperr);
return 0;
}

static int zero (lua_State* L)
{
  const SmipsBank* self = luaL_checkudata (L, 1, META);
  const guint size = luaL_checkinteger (L, 2);
  GError* tmperr = NULL;

  g_assert (size % 4 == 0);
  g_seekable_seek (self->seekable, size, G_SEEK_CUR, NULL, &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 0, tmperr);
return 0;
}

static int emit8 (lua_State* L)
{
  const SmipsBank* self = luaL_checkudata (L, 1, META);
  const guint8 value = luaL_checkinteger (L, 2);
  GError* tmperr = NULL;

  g_output_stream_write_all (self->stream, &value, sizeof (value), NULL, NULL, &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 0, tmperr);
return 0;
}

static int emit16 (lua_State* L)
{
  const SmipsBank* self = luaL_checkudata (L, 1, META);
  const guint16 other = luaL_checkinteger (L, 2);
  const guint16 value = GUINT16_TO_LE (other);
  GError* tmperr = NULL;

  g_output_stream_write_all (self->stream, &value, sizeof (value), NULL, NULL, &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 0, tmperr);
return 0;
}

static int emit32 (lua_State* L)
{
  const SmipsBank* self = luaL_checkudata (L, 1, META);
  const guint32 other = luaL_checkinteger (L, 2);
  const guint32 value = GUINT32_TO_LE (other);
  GError* tmperr = NULL;

  g_output_stream_write_all (self->stream, &value, sizeof (value), NULL, NULL, &tmperr);

  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 0, tmperr);
return 0;
}

static int emits (lua_State* L)
{
  size_t size;
  const SmipsBank* self = luaL_checkudata (L, 1, META);
  const char* value = luaL_checklstring (L, 2, &size);
  GError* tmperr = NULL;

  g_output_stream_write_all (self->stream, value, size, NULL, NULL, &tmperr);
  lua_pop (L, 1);

  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 0, tmperr);
return 0;
}

G_MODULE_EXPORT
int luaopen_banks (lua_State* L)
{
  lua_createtable (L, 0, 6);
  luaL_newmetatable (L, META);
#if LUA_VERSION_NUM < 503
  lua_pushliteral (L, META);
  lua_setfield (L, -2, "__name");
#endif // LUA_VERSION_NUM
  lua_pushcfunction (L, __gc);
  lua_setfield (L, -2, "__gc");
  lua_pushvalue (L, -2);
  lua_setfield (L, -2, "__index");
  lua_pop (L, 1);

  lua_pushcfunction (L, _new);
  lua_setfield (L, -2, "new");
  lua_pushcfunction (L, _close);
  lua_setfield (L, -2, "close");
  lua_pushcfunction (L, zero);
  lua_setfield (L, -2, "zero");
  lua_pushcfunction (L, emit8);
  lua_setfield (L, -2, "emit8");
  lua_pushcfunction (L, emit32);
  lua_setfield (L, -2, "emit32");
  lua_pushcfunction (L, emits);
  lua_setfield (L, -2, "emits");
return 1;
}
