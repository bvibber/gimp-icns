= icns plugin for Gimp 2 =

Gimp file format plug-in for Macintosh icon files (.icns)
   
   by Brion Vibber <brion at pobox.com>
   http://leuksman.com/mac/gimp/
   
   v0.1, 2004-07-24
   GPL license; see COPYING.txt

Based on winicon, Copyright (C) 2002 Christian Kreibich <christian@whoop.org>.


Currently read-only; can load true color, 8-bit, 4-bit, and 1-bit icons at
128x128, 48x48, 32x32, and 16x16 sizes. Multiple images are loaded as separate
layers.


== Installation ==

The distribution tarball includes source only; it is cross platform so should
work so long as you've got the appropriate developer tools and Gimp devel
packages.

Compiling the plugin requires that you have the Developer Tools installed and
have either installed Gimp from source or have installed the dev package with
the necessary header files.

To compile & install into ~/.gimp-2.0/plug-ins:
  make install

To compile only:
  make


== Version history ==

Version 0.1, 2004-07-24
* Initial release; read-only.
