/* GIMP Plug-in for Macintosh icns icon resource files.
 * Copyright (C) 2004 Brion Vibber <brion@pobox.com>
 *
 * Based in part on Winicon,
 * Copyright (C) 2002 Christian Kreibich <christian@whoop.org>.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __MAIN_H__
#define __MAIN_H__

extern gboolean   interactive_ico;

#ifdef ICO_DBG
#define D(x) \
{ \
  printf("ICNS plugin: "); \
  printf x; \
}
#else
#define D(x)
#endif

typedef struct _ResourceHeader
{
  /* Big-endian! */
  gint32  type;
  guint32 size;
} ResourceHeader;

typedef struct _Resource
{
  gint32  type;
  guint32 size;
  guint32 cursor;
  guchar *data;
} Resource;

#define SANITY_CHECK (128*128*5*10)

void fourcc_get_string (gint32 fourcc, gchar *buf);
void hexdump (guchar *data, guint nbytes);

#endif
