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
#ifndef __SMIPS__
#define __SMIPS__ 1
#include <gio/gio.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#define GRESROOT "/org/hck/smips"

G_BEGIN_DECLS

extern GResource* bundle_get_resource (void);
G_GNUC_INTERNAL int gerror (lua_State* L, const char* loc, GError* error) G_GNUC_NORETURN;
G_GNUC_INTERNAL int pload (lua_State* L, const gchar* path);
G_GNUC_INTERNAL int rloader (lua_State* L);
G_GNUC_INTERNAL int bloader (lua_State* L);

#define gerror(error) (gerror) (L, G_STRLOC, ((error)))

G_END_DECLS

#endif // __SMIPS__
