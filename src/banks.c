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
#include <banks.h>
#include <gio/gio.h>
#include <gmodule.h>
#include <log.h>

#define META "SmipsBank"

static int __gc (lua_State* L)
{
  SmipsBank* self = luaL_checkudata (L, 1, META);
    g_clear_object (&self->stream);
return 0;
}

static int _new (lua_State* L)
{
  const gsize sz = sizeof (SmipsBank);
  const gchar* name = luaL_checkstring (L, 1);
  SmipsBank* self = lua_newuserdata (L, sz);
#if LUA_VERSION_NUM >= 502
  luaL_setmetatable (L, META);
#else // LUA_VERSION_NUM
  lua_getfield (L, LUA_REGISTRYINDEX, META);
  lua_setmetatable (L, -2);
#endif // LUA_VERSION_NUM

  GFile* file;
  GOutputStream* stream;
  GError* tmperr = NULL;

  file = (gpointer) g_file_new_for_commandline_arg (name);
  stream = (gpointer) g_file_replace (file, NULL, 0, 0, NULL, &tmperr);
  self->stream = (void*) stream;
  g_object_unref (file);

  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 1, tmperr);
return 1;
}

static int _emit (lua_State* L)
{
  const SmipsBank* self = luaL_checkudata (L, 1, META);
  const guint32 value = luaL_checkinteger (L, 2);

  gchar buf [16] = {0};
  gint bufsz = g_snprintf (buf, sizeof (buf), "%08x\n", GUINT32_TO_BE (value));
  GError* tmperr = NULL;

  g_assert (bufsz < sizeof (buf));
  g_output_stream_write_all (self->stream, buf, bufsz, NULL, NULL, &tmperr);
  if (G_UNLIKELY (tmperr != NULL))
    _smips_log_gerror (L, 1, tmperr);
return 0;
}

G_MODULE_EXPORT
int luaopen_banks (lua_State* L)
{
  lua_createtable (L, 0, 2);
  luaL_newmetatable (L, META);
  lua_pushcfunction (L, __gc);
  lua_setfield (L, -2, "__gc");
  lua_pushvalue (L, -2);
  lua_setfield (L, -2, "__index");
  lua_pop (L, 1);

  lua_pushcfunction (L, _new);
  lua_setfield (L, -2, "new");
  lua_pushcfunction (L, _emit);
  lua_setfield (L, -2, "emit");
return 1;
}
