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
#ifndef __SMIPS_TAGS__
#define __SMIPS_TAGS__ 1
#include <glib.h>
#include <lua.h>
#include <lauxlib.h>
#include <luacmpt.h>

typedef struct _SmipsTag SmipsTag;
typedef struct _SmipsTagOp SmipsTagOp;

#if __cplusplus
extern "C" {
#endif // __cplusplus

enum
{
  TAG_VALUE = 1,
  TAG_REL = (1 << 1),

  TAG_OPER = 0,
  TAG_ADD = (1 << 1),
  TAG_SUB = (2 << 1),
  TAG_MUL = (3 << 1),
  TAG_DIV = (4 << 1),
  TAG_IDIV = (5 << 1),
  TAG_MOD = (6 << 1),
  TAG_UNM = (7 << 1),

  TAG_VALUE_MASK = (TAG_REL),
  TAG_OP_MASK = (TAG_ADD | TAG_SUB | TAG_MUL| TAG_DIV | TAG_IDIV | TAG_MOD| TAG_UNM),
};

struct _SmipsTag
{
  guint refs;
  int type;

  union
  {
    guint value;

    struct
    {
      SmipsTag* left;
      SmipsTag* right;
    };
  };
};

#if __cplusplus
}
#endif // __cplusplus

#endif // __SMIPS_TAGS__
