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

//#include "config.h"

#include <errno.h>
#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include <libgimp/gimp.h>

#define ICO_DBG

#include "main.h"
#include "icnsdata.h"

//#include "libgimp/stdplugins-intl.h"
//#include <libintl.h>
//#define N_(x) _(x)

Resource *
resource_load(const gchar *filename)
{
  Resource *res = NULL;
  FILE *file;
  
  file = fopen (filename, "rb");
  if (file)
    {
      ResourceHeader header;
      if (1 == fread (&header, sizeof (ResourceHeader), 1, file))
        {
          gint32 type;
          guint32 size;
          type = GINT32_FROM_BE (header.type);
          size = GUINT32_FROM_BE (header.size);
          if (type == 'icns' && size > sizeof (ResourceHeader) && size < SANITY_CHECK)
            {
              res = (Resource *)g_new (guchar, sizeof (Resource) + size);
              res->type = type;
              res->size = size;
              res->cursor = sizeof (ResourceHeader);
              res->data = (guchar *)res + sizeof (Resource);
              fseek (file, 0, SEEK_SET);
              if (size != fread (res->data, 1, res->size, file))
                {
                  D(("** expected %d bytes\n", size));
                  g_free (res);
                  res = NULL;
                }
            }
          else
            {
              char typestring[5];
              fourcc_get_string (type, typestring);
              D(("** resource type '%s' or size '%d' seems out of whack\n",
                typestring, size));
            }
        }
      else
        {
          D(("** couldn't read header.\n"));
        }
      fclose (file);
    }
  else
    {
      D(("** couldn't open file.\n"));
    }
  return res;
}

gboolean
resource_getnext(Resource *icns, Resource *res)
{
  ResourceHeader *header;
  if (icns->size - icns->cursor < sizeof (ResourceHeader))
    return FALSE;

  header = (ResourceHeader *)&(icns->data[icns->cursor]);
  res->type = GINT32_FROM_BE (header->type);
  res->size = GUINT32_FROM_BE (header->size);
  res->cursor = sizeof (ResourceHeader);
  res->data = &(icns->data[icns->cursor]);
  
  icns->cursor += res->size;
  if (icns->cursor > icns->size)
    {
      guchar typestring[5];
      fourcc_get_string(icns->type, typestring);
      g_message ("icns resource_getnext: resource too big! type '%s', size %u\n",
                 typestring, icns->size);
      return FALSE;
    }
  return TRUE;
}

Resource *
resource_find (Resource *list, gint32 type, gint max)
{
	int i;
	for (i = 0; i < max; i++)
	  {
	    if (list[i].type == type)
	      return &list[i];
	  }
	return NULL;
}

void
icns_slurp (guchar *dest, IconType *icontype, Resource *image, Resource *mask)
{
  guint out, max, row;
  guchar bucket;
  guchar bit;
  guint index;
  
  row = icontype->width * 4;
  max = icontype->width * icontype->height;
  image->cursor = sizeof (ResourceHeader);
  switch (icontype->bits)
    {
    case 1:
      for (out = 0; out < max; out++)
        {
          if (out % 8 == 0)
            bucket = image->data[image->cursor++];
          bit = (bucket & 0x80) ? 0 : 255;
          bucket = bucket << 1;
          dest[out * 4    ] = bit;
          dest[out * 4 + 1] = bit;
          dest[out * 4 + 2] = bit;
        }
      break;
    case 4:
      for (out = 0; out < max; out++)
        {
          if (out % 2 == 0)
            bucket = image->data[image->cursor++];
          index = 3 * (bucket & 0xf0) >> 4;
          bucket = bucket << 4;
          dest[out * 4    ] = icns_colormap_4[index    ];
          dest[out * 4 + 1] = icns_colormap_4[index + 1];
          dest[out * 4 + 2] = icns_colormap_4[index + 2];
        }
      break;
    case 8:
      for (out = 0; out < max; out++)
        {
          index = 3 * image->data[image->cursor++];
          dest[out * 4    ] = icns_colormap_8[index    ];
          dest[out * 4 + 1] = icns_colormap_8[index + 1];
          dest[out * 4 + 2] = icns_colormap_8[index + 2];
          dest[out * 4 + 3] = 255;
        }
      break;
    case 32:
      for (out = 0; out < max; out++)
        {
          dest[out * 4    ] = image->data[image->cursor++];
          dest[out * 4 + 1] = image->data[image->cursor++];
          dest[out * 4 + 2] = image->data[image->cursor++];
          // Throw away alpha, use the mask
          image->cursor++;
          if (mask)
            dest[out * 4 + 3] = image->data[mask->cursor++];
          else
            dest[out * 4 + 3] = 255;
        }
      break;
    }
    
    /* Now for the mask */
    if (mask && icontype->bits != 32)
      {
        mask->cursor = sizeof (ResourceHeader) + icontype->width * icontype->height / 8;
        for (out = 0; out < max; out++)
          {
            if (out % 8 == 0)
                bucket = mask->data[mask->cursor++];
            bit = (bucket & 0x80) ? 255 : 0;
            bucket = bucket << 1;
            dest[out * 4 + 3] = bit;
          }
      }
}

gboolean
icns_decompress (guchar *dest, IconType *icontype, Resource *image, Resource *mask)
{
  guint x, y, row, max;
  guint channel, out;
  guchar run, val;
  
  max = icontype->width * icontype->height;
  memset(dest, 255, max*4);

  /* For some reason there seem to be 4 null bytes at the start of an it32. */
  if (icontype->type == 'it32')
    image->cursor += 4;
  
  for (channel = 0; channel < 3; channel++)
    {
      out = 0;
      g_print ("icns_decompress: channel %d at in %d\n",
        channel, image->cursor, out);
      while (out < max)
        {
          //g_print("Hi! out %d, max %d, cursor %d\n", out, max, image->cursor);
          run = image->data[image->cursor++];
          //g_print("[%d] run byte 0x%02x: ", image->cursor-1, (guint)run);
          //g_print("Hi! out %d, max %d, cursor %d, run %d\n", out, max, (guint)run, image->cursor);
          if (run & 0x80)
            {
              // compressed
              if (image->cursor >= image->size)
                {
                  g_message ("Corrupt icon: compressed run overflows input size.");
                  return FALSE;
                }
              val = image->data[image->cursor++];
              //g_print ("icns_decompress: compressed run of %d from output %d, value %d\n", (guint)(run - 125), out, (guint)val);
              for (run -= 125; run > 0; run--)
                {
                  if (out >= max)
                    {
                      g_message ("Corrupt icon? compressed run overflows output size.");
                      return FALSE;
                    }
                  dest[out++ * 4 + channel] = val;
                }
            }
          else
            {
              // uncompressed
              //g_print ("icns_decompress: uncompressed run of %d from output %d\n", (guint)(run + 1), out);
              for (run += 1; run > 0; run--)
                {
                  if (image->cursor >= image->size)
                    {
                      g_message ("Corrupt icon: uncompressed run overflows input size.");
                      return FALSE;
                    }
                  if (out >= max)
                    {
                      g_message ("Corrupt icon: uncompressed run overflows output size.");
                      return FALSE;
                    }
                  dest[out++ * 4 + channel] = image->data[image->cursor++];
                }
            }
        }
      }
    
    if (mask)
      {
        gchar typestring[5];
        fourcc_get_string (mask->type, typestring);
        g_print("icns_decompress: using 8-bit alpha mask type %s\n", typestring);
        for (out = 0; out < max; out++)
          dest[out * 4 + 3] = mask->data[mask->cursor++];
      }
    return TRUE;
}

void
icns_attach_image (guint32 image_ID, IconType *icontype, Resource *image, Resource *mask)
{
  gchar typestring[5];
  guchar         *dest;
  GimpDrawable   *drawable;
  gint32 layer;
  GimpPixelRgn pixel_rgn;
  guint x, y, row, expected_size;
   
  fourcc_get_string (icontype->type, typestring);
  g_print ("icnsload: bit depth %d loading (type '%s')\n",
           icontype->bits, typestring);
  
  layer = gimp_layer_new (image_ID, typestring, icontype->width, icontype->height,
                              GIMP_RGBA_IMAGE, 100, GIMP_NORMAL_MODE);

  gimp_image_add_layer (image_ID, layer, -1);
  drawable = gimp_drawable_get (layer);
  row = 4 * icontype->width;
  dest = g_malloc (row * icontype->height);
  
  expected_size =
    (icontype->width * icontype->height * icontype->bits) / 8;
  if (image == mask)
    expected_size *= 2;
  expected_size += sizeof (ResourceHeader);
  
  if (icontype->bits != 32 || expected_size == image->size)
    {
      icns_slurp (dest, icontype, image, mask);
    }
  else
    {
      g_print( "Compressed! (expected %d, got %d bytes)\n", expected_size, image->size);
      icns_decompress (dest, icontype, image, mask);
    }
  
  // ....
  gimp_pixel_rgn_init (&pixel_rgn, drawable,
                       0, 0, drawable->width, drawable->height,
                       TRUE, FALSE);
  gimp_pixel_rgn_set_rect (&pixel_rgn, dest,
                           0, 0, drawable->width, drawable->height);
  g_free (dest);
  gimp_drawable_flush (drawable);
  gimp_drawable_detach (drawable);
}

gint32
icns_load (Resource *icns, const gchar *filename)
{
  Resource *resources;
  gchar typestring[5];
  guint i, nResources;
  guint32 image_ID;
  
  resources = g_new (Resource, 256);
  
  image_ID = gimp_image_new (128, 128, GIMP_RGB);
  gimp_image_set_filename (image_ID, filename);

  nResources = 0;
  while (resource_getnext (icns, &resources[nResources++]))
    {
#ifdef ICO_DBG
      for (i = 0; iconTypes[i].type; i++)
        if (resources[nResources-1].type == iconTypes[i].type)
          continue;
      for (i = 0; maskTypes[i].type; i++)
        if (resources[nResources-1].type == maskTypes[i].type)
          continue;
      fourcc_get_string (resources[nResources-1].type, typestring);
      g_print ("icns_load: Unknown resources '%s', %d bytes:\n", typestring, resources[nResources-1].size);
      hexdump (resources[nResources-1].data, resources[nResources-1].size);
#endif
    }
  
  for (i = 0; iconTypes[i].type; i++)
    {
      Resource *image;
      Resource *mask;
      if (image = resource_find (resources, iconTypes[i].type, nResources))
        {
          mask = resource_find (resources, iconTypes[i].mask, nResources);
          icns_attach_image (image_ID, &iconTypes[i], image, mask);
        }
    }

  g_free (resources);
  return image_ID;
}

gint32
LoadICNS (const gchar *filename)
{
  gint32 image_ID;
  Resource *icns;

  D(("*** got to LoadICNS('%s')\n", filename));
  
  if (interactive_ico)
    {
      guchar *temp = g_strdup_printf ("Loading %s:",
                                      gimp_filename_to_utf8 (filename));
      gimp_progress_init (temp);
      g_free (temp);
    }

  icns = resource_load (filename);
  if (! icns)
    {
      g_message ("Invalid or corrupt icns resource file.");
      return -1;
    }
  
  image_ID = icns_load (icns, filename);

  g_free (icns);
  D(("*** icon successfully loaded.\n\n"));
  return image_ID;
}
