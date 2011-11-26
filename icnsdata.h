#ifndef _ICNS_DATA_H
#define _ICNS_DATA_H

typedef struct _IconType
{
  gint32   type;
  guint    width;
  guint    height;
  guint    bits;
  gint32   mask;
} IconType;

extern IconType iconTypes[];
extern IconType maskTypes[];
extern guchar   icns_colormap_4[];
extern guchar   icns_colormap_8[];

#endif
