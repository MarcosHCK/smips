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

typedef struct _SmipsRaw2Stream SmipsRaw2Stream;
typedef struct _SmipsRaw2StreamClass SmipsRaw2StreamClass;
#define __sep "\r\n"
#define __header "v2.0 raw" __sep
#define __charset "0123456789abcdef"
#define __headersz (sizeof (__header) - 1)
#define __sepsz (sizeof (__sep) - 1)
#define __align (4)
#define __wordsz (__align << 1)
#define __bufsz (__wordsz + __sepsz)

struct _SmipsRaw2Stream
{
  GBufferedOutputStream parent;

  /* private */
  guint8 albuf [__bufsz];
  gint wrote;
  gint presented;
};

struct _SmipsRaw2StreamClass
{
  GBufferedOutputStreamClass parent;
};

G_DEFINE_FINAL_TYPE (SmipsRaw2Stream, smips_raw2_stream, G_TYPE_BUFFERED_OUTPUT_STREAM);
G_STATIC_ASSERT ((G_MAXINT >> 1) > __align);

static gssize smips_raw2_stream_class_write_fn (GOutputStream* pself, const void* _buffer, gsize count, GCancellable* cancellable, GError** error)
{
  SmipsRaw2Stream* self = (gpointer) pself;
  int i, left = __align - self->wrote;
  const guint8* buffer = _buffer;
  GError* tmperr = NULL;
  gsize wrote = 0;

  if (self->presented == 0)
  {
    ++self->presented;

    guint8* __buffer = __header;
    gsize __size = __headersz;
    gsize __wrote = 0;
    gsize got;

    while (__wrote < __size)
    {
      if ((got = G_OUTPUT_STREAM_CLASS (smips_raw2_stream_parent_class)->write_fn (pself, __buffer + __wrote, __size - __wrote, cancellable, &tmperr)) > 0)
        __wrote += got;
      else
      {
        g_propagate_error (error, tmperr);
        return -1;
      }
    }
  }

  do
  {
    left = MIN (count, left);

    for (i = 0; i < left; i++)
    {
      guint8 value = *buffer++;
      self->albuf [self->wrote + (i << 1) + 0] = __charset [value >> 4];
      self->albuf [self->wrote + (i << 1) + 1] = __charset [value & 0xf];
    }

    self->wrote += left;

    if (self->wrote == __align)
    {
      guint8* __buffer = self->albuf;
      gsize __size = G_SIZEOF_MEMBER (SmipsRaw2Stream, albuf);
      gsize __wrote = 0;
      gsize got;

      memcpy (& self->albuf [__wordsz], __sep, __sepsz);
      self->wrote = 0;

      while (__wrote < __size)
      {
        if ((got = G_OUTPUT_STREAM_CLASS (smips_raw2_stream_parent_class)->write_fn (pself, __buffer + __wrote, __size - __wrote, cancellable, &tmperr)) > 0)
          __wrote += got;
        else
        {
          g_propagate_error (error, tmperr);
          return -1;
        }
      }
    }

    wrote += left;
    count -= left;
  }
  while (count > 0);
return wrote;
}

static void smips_raw2_stream_class_constructed (GObject* pself)
{
  SmipsRaw2Stream* self = (gpointer) pself;
G_OBJECT_CLASS (smips_raw2_stream_parent_class)->constructed (pself);
}

static void smips_raw2_stream_class_init (SmipsRaw2StreamClass* klass)
{
  GOutputStreamClass* sclass = G_OUTPUT_STREAM_CLASS (klass);
  GObjectClass* oclass = G_OBJECT_CLASS (klass);

  sclass->write_fn = smips_raw2_stream_class_write_fn;
  oclass->constructed = smips_raw2_stream_class_constructed;
}

static void smips_raw2_stream_init (SmipsRaw2Stream* self)
{
}
