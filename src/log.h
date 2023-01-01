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
#ifndef __SMIPS_LOG__
#define __SMIPS_LOG__ 1
#include <glib.h>
#include <lua.h>
#include <lualib.h>
#include <lauxlib.h>

#if __cplusplus
extern "C" {
#endif // __cplusplus

G_GNUC_INTERNAL int _smips_msgh (lua_State* L);
G_GNUC_INTERNAL int _smips_log_error (lua_State* L, int level, int user) G_GNUC_NORETURN;
G_GNUC_INTERNAL int _smips_log_lerror (lua_State* L, int user, const gchar* message) G_GNUC_NORETURN;
G_GNUC_INTERNAL int _smips_log_gerror (lua_State* L, int user, GError* error) G_GNUC_NORETURN;

#if __cplusplus
}
#endif // __cplusplus

#endif // __SMIPS_LOG__
