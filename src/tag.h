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
#ifndef __SMIPS_TAG__
#define __SMIPS_TAG__ 1
#include <tags.h>

typedef struct _SmipsTagIndex SmipsTagIndex;

#if __cplusplus
extern "C" {
#endif // __cplusplus

struct _SmipsTagIndex
{
  int name;
  int type;
  int validin;
  goffset offset;
};

G_GNUC_INTERNAL void *_smips_tag_new ();
G_GNUC_INTERNAL void *_smips_tag_ref (gpointer ptag);
G_GNUC_INTERNAL void _smips_tag_unref (gpointer ptag);
G_GNUC_INTERNAL void _smips_tag_type (const SmipsTag* tag, const gchar** _type, const gchar** _subtype);
G_GNUC_INTERNAL void _smips_tag_print (const SmipsTag* tag, int level);
G_GNUC_INTERNAL const SmipsTagIndex* _smips_tag_index_lookup (const char *str, size_t len);

#if __cplusplus
}
#endif // __cplusplus

#endif // __SMIPS_TAG__
