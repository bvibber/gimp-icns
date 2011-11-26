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

#include <string.h>
#include <stdio.h>

#include <gtk/gtk.h>

#include <libgimp/gimp.h>
#include <libgimp/gimpui.h>

/* #define ICO_DBG */

#include "main.h"
#include "icnsdata.h"
#include "icnsload.h"
#include "icnssave.h"

//#include "libgimp/stdplugins-intl.h"
#include <libintl.h>
#define N_(x) _(x)

gboolean interactive_ico = FALSE;

static void   query (void);
static void   run   (const gchar      *name,
                     gint              nparams,
                     const GimpParam  *param,
                     gint             *nreturn_vals,
                     GimpParam       **return_vals);


GimpPlugInInfo PLUG_IN_INFO =
{
  NULL,  /* init_proc  */
  NULL,  /* quit_proc  */
  query, /* query_proc */
  run,   /* run_proc   */
};

MAIN ()

static void
query (void)
{
  static GimpParamDef load_args[] =
  {
    { GIMP_PDB_INT32,    "run_mode",     "Interactive, non-interactive" },
    { GIMP_PDB_STRING,   "filename",     "The name of the file to load" },
    { GIMP_PDB_STRING,   "raw_filename", "The name entered"             }
  };
  static GimpParamDef load_return_vals[] =
  {
    { GIMP_PDB_IMAGE, "image", "Output image" },
  };

  static GimpParamDef save_args[] =
  {
    { GIMP_PDB_INT32,    "run_mode",     "Interactive, non-interactive" },
    { GIMP_PDB_IMAGE,    "image",        "Input image" },
    { GIMP_PDB_DRAWABLE, "drawable",     "Drawable to save" },
    { GIMP_PDB_STRING,   "filename",     "The name of the file to save the image in" },
    { GIMP_PDB_STRING,   "raw_filename", "The name entered" },
  };

  gimp_install_procedure ("file_icns_load",
                          "Loads files of Macintosh ICNS file format",
                          "Loads files of Macintosh ICNS file format",
                          "Christian Kreibich <christian@whoop.org>",
                          "Christian Kreibich <christian@whoop.org>",
                          "2002",
                          "<Load>/icns",
                          NULL,
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (load_args),
                          G_N_ELEMENTS (load_return_vals),
                          load_args, load_return_vals);

  gimp_register_magic_load_handler ("file_icns_load",
                                    "icns",
                                    "",
                                    "0,string,icns");

  gimp_install_procedure ("file_icns_save",
                          "Saves files in Macintosh ICNS file format",
                          "Saves files in Macintosh ICNS file format",
                          "Brion Vibber",
                          "Brion Vibber",
                          "2004",
                          "<Save>/icns",
                          "INDEXEDA, GRAYA, RGBA",
                          GIMP_PLUGIN,
                          G_N_ELEMENTS (save_args), 0,
                          save_args, NULL);

  gimp_register_save_handler ("file_icns_save",
                              "icns",
                              "");
}

static void
run (const gchar      *name,
     gint              nparams,
     const GimpParam  *param,
     gint             *nreturn_vals,
     GimpParam       **return_vals)
{
  static GimpParam   values[2];
  gint32             image_ID;
  gint32             drawable_ID;
  GimpRunMode        run_mode;
  GimpPDBStatusType  status = GIMP_PDB_SUCCESS;
  GimpExportReturn   export = GIMP_EXPORT_CANCEL;

  run_mode = param[0].data.d_int32;

  *nreturn_vals = 1;
  *return_vals  = values;
  values[0].type          = GIMP_PDB_STATUS;
  values[0].data.d_status = GIMP_PDB_EXECUTION_ERROR;

  if (strcmp (name, "file_icns_load") == 0)
    {
      switch (run_mode)
        {
        case GIMP_RUN_INTERACTIVE:
          interactive_ico = TRUE;
          break;

        case GIMP_RUN_NONINTERACTIVE:
          interactive_ico = FALSE;
          if (nparams != 3)
            status = GIMP_PDB_CALLING_ERROR;
          break;

        default:
          break;
        }

      if (status == GIMP_PDB_SUCCESS)
        {
          image_ID = LoadICNS (param[1].data.d_string);

           if (image_ID != -1)
             {
               *nreturn_vals = 2;
               values[1].type         = GIMP_PDB_IMAGE;
               values[1].data.d_image = image_ID;
             }
           else
             {
               status = GIMP_PDB_EXECUTION_ERROR;
             }
        }
    }
  else if (strcmp (name, "file_icns_save") == 0)
    {
      gchar *file_name;

      image_ID    = param[1].data.d_int32;
      drawable_ID = param[2].data.d_int32;
      file_name   = param[3].data.d_string;

      switch (run_mode)
        {
        case GIMP_RUN_INTERACTIVE:
          interactive_ico = TRUE;
          break;

        case GIMP_RUN_NONINTERACTIVE:
          interactive_ico = FALSE;
          /*  Make sure all the arguments are there!  */
          if (nparams < 5)
            status = GIMP_PDB_CALLING_ERROR;
          break;

        case GIMP_RUN_WITH_LAST_VALS:
          interactive_ico = FALSE;
          break;

        default:
          break;
        }

      if (status == GIMP_PDB_SUCCESS)
        {
          status = SaveICNS (file_name, image_ID);
        }

      if (export == GIMP_EXPORT_EXPORT)
        gimp_image_delete (image_ID);
    }
  else
    {
      status = GIMP_PDB_CALLING_ERROR;
    }

  values[0].type = GIMP_PDB_STATUS;
  values[0].data.d_status = status;
}

/* Buffer should point to *at least 5 byte buffer*! */
void
fourcc_get_string (gint32 fourcc, gchar *buf)
{
  *((gint32 *)buf) = GINT32_TO_BE (fourcc);
  buf[4] = 0;
}

void hexdump (guchar *data, guint nbytes)
{
  guint i;
  for (i = 0; i < nbytes; i++)
    {
      g_print ("%02x%c", data[i], (i % 16) ? ' ' : '\n');
    }
  if (nbytes % 16)
    g_print ("\n");
}
