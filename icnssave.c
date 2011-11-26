#include <stdio.h>

#include <libgimp/gimp.h>
#include "icnsdata.h"
#include "main.h"

#define icns_debug g_print

guchar *icns_get_alpha (guint width, guint height, guchar *pixels)
{
  // Assuming RGBA
  guchar *alpha;
  guint   npixels;
  
  npixels = width * height;
  alpha = g_new (guchar, npixels);
  if (alpha)
    {
      guint i;
      for (i = 0; i < npixels; i++)
        alpha[i] = pixels[i * 4 + 3];
    }
  return alpha;
}


/*
 * Each color channel (red, green, blue) is treated as a single line of pixels.
 * Runs of same-value pixels are stored as (count - 3) | 128 followed by the value.
 * Runs of different-value pixels are stored as (count - 1) followed by the series of values;
 */
guchar *icns_compress (guint width, guint height, guchar *rgba, guint *out_size)
{
  const guint npixels = width * height;
  const guint max_size = width * height * 3 * (130 / 128) + 1; // ?

  const guint min_run = 3; /* Shorter run must be stored as uncompressed */
  const guint max_run = 130; /* Longest same-value run that can be stored */
  const guint min_raw = 1;
  const guint max_raw = 128; /* Longest run of non-matching pixels */
  
  guint i, j, size, channel, run, marker;
  guchar *out_data;
  guchar *run_length;
  
  run_length = g_new (guchar, npixels);
  if (!run_length)
    {
      g_warning ("icns_compress: couldn't allocate run count buffer (%d bytes)", npixels);
      return NULL;
    }
  
  out_data = g_new (guchar, max_size);
  if (!out_data)
    {
      g_warning ("icns_compress: couldn't allocate output buffer (%d bytes)", max_size);
      g_free (run_length);
      return NULL;
    }
  
  size = 0;
  /* For some reason 128x128 icons have an extra 4 bytes at the start */
  if (width == 128 && height == 128)
    {
      out_data[size++] = 0;
      out_data[size++] = 0;
      out_data[size++] = 0;
      out_data[size++] = 0;
    }
  
  for (channel = 0; channel < 3; channel++)
    {
      icns_debug ("Channel %d at output %d\n", channel, size);
      
      /* Count all run lengths */
      for (i = 0; i < npixels; i++)
        {
          for (run = 1; run < max_run && run + i - 1 < npixels ; run++)
            if (rgba[i * 4 + channel] != rgba[(i + run) * 4 + channel])
              break;
          run_length[i] = run;
        }
      for (i = 0; i < npixels; i++)
        {
          icns_debug ("[%6d] ", i);
          if (run_length[i] >= min_run)
            {
              /* Compressable! Store and skip ahead */
              icns_debug ("Compressed run of %d pixels at %d\n", run_length[i], size);
              out_data[size++] = (run_length[i] - min_run) | 0x80;
              out_data[size++] = rgba[i * 4 + channel];
              i += run_length[i] - 1;
            }
          else
            {
              /* Too short: stuff together as many as you can in a raw run */
              marker = size++;
              run = 0;
              while (run < max_raw && i < npixels && run_length[i] < min_run)
                {
                  for (j = 0; j < run_length[i]; j++)
                    {
                      out_data[size++] = rgba[(i + j) * 4 + channel];
                      run++;
                    }
                  i += run_length[i];
                }
              out_data[marker] = run - min_raw;
              icns_debug ("Uncompressed run of %d pixels at %d\n", run, marker);
              i--; /* Overcorrected */
            }
        }
    }

  g_free (run_length);
  *out_size = size;
  return out_data;
}


GimpPDBStatusType
SaveICNS (const gchar *file_name,
          gint32       image_ID)
{
  // Horrible awful temporary hack
  ResourceHeader icnsHeader;
  ResourceHeader it32Header;
  ResourceHeader t8mkHeader;
  
  guchar *compData, *alphaData;
  FILE *outfile;
  
  guchar *pixels;
  gint dataSize;
  

  if (gimp_image_base_type (image_ID) != GIMP_RGB)
    {
      if (! gimp_image_convert_rgb (image_ID))
        return GIMP_PDB_EXECUTION_ERROR;
    }

  gint *layers;
  gint nlayers;
  GimpPixelRgn region;
  
  layers = gimp_image_get_layers (image_ID, &nlayers);
  GimpDrawable *drawable = gimp_drawable_get (layers[0]);
  
  pixels = g_new (guchar, 128*128*4);
  gimp_pixel_rgn_init (&region, drawable, 0, 0, 128, 128, TRUE, FALSE);
  gimp_pixel_rgn_get_rect (&region, pixels, 0, 0, 128, 128);
  
  compData = icns_compress (128, 128, pixels, &dataSize);
  alphaData = icns_get_alpha (128, 128, pixels);
  
  it32Header.type = GUINT32_TO_BE ('it32');
  it32Header.size = GUINT32_TO_BE (sizeof (ResourceHeader) + dataSize);
  
  t8mkHeader.type = GUINT32_TO_BE ('t8mk');
  t8mkHeader.size = GUINT32_TO_BE (sizeof (ResourceHeader) + 128*128);
  
  icnsHeader.type = GUINT32_TO_BE ('icns');
  icnsHeader.size = dataSize + 128*128 + 3 * sizeof (ResourceHeader);
  
  outfile = fopen (file_name, "wb");
  if (outfile)
    {
      fwrite (&icnsHeader, 1, sizeof (ResourceHeader), outfile);
      fwrite (&it32Header, 1, sizeof (ResourceHeader), outfile);
      fwrite (compData, 1, dataSize, outfile);
      fwrite (&t8mkHeader, 1, sizeof (ResourceHeader), outfile);
      fwrite (alphaData, 1, 128*128, outfile);
      fclose (outfile);
    }
  else
    {
      g_warning ("SaveICNS: couldn't open output file");
    }

  g_free (pixels);
  g_free (compData);
  g_free (alphaData);
  return GIMP_PDB_SUCCESS;
}
