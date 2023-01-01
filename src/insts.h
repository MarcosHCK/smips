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
#ifndef __SMIPS_INSTS__
#define __SMIPS_INSTS__ 1
#include <glib.h>
#include <lua.h>
#include <lauxlib.h>

typedef struct _SmipsInst SmipsInst;

#if __cplusplus
extern "C" {
#endif // __cplusplus

struct _SmipsInst
{
  int type;
  guint opcode;
  guint constant;
  guint shamt;
  guint func;
  guint rd, rs, rt;
  gchar* cs;
};

enum
{
  R_INST = (1 << 0),
  I_INST = (1 << 1),
  J_INST = (1 << 2),
  MASK_INST = (R_INST | I_INST | J_INST),
};

#if __cplusplus
}
#endif // __cplusplus

#endif // __SMIPS_INSTS__
