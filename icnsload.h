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

#ifndef __ICO_LOAD_H__
#define __ICO_LOAD_H__

gint32 LoadICNS                  (const gchar *file_name);

#define ICO_DBG 1

#endif
