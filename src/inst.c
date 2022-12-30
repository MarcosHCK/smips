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
#include <inst.h>
#include <smips.h>

typedef struct _Template Template;

struct _Template
{
  GHashTable* attrs;
};

G_MODULE_EXPORT
int luaopen_inst (lua_State* L)
{
  luaL_newmetatable (L, R_INST);
  lua_pop (L, 1);
  luaL_newmetatable (L, I_INST);
  lua_pop (L, 1);
  luaL_newmetatable (L, J_INST);
  lua_pop (L, 1);

  lua_createtable (L, 0, 0);

  lua_createtable (L, 0, 0);
  lua_setfield (L, -2, "r_inst");
  lua_createtable (L, 0, 0);
  lua_setfield (L, -2, "i_inst");
  lua_createtable (L, 0, 0);
  lua_setfield (L, -2, "j_inst");
return 1;
}
