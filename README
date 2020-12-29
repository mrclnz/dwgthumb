This is an attempt to make a working DWG thumbnailer for
gnome/freedesktop tumbnail processor. As in, those using the
/usr/share/thumbnailers directory to know how to generate a thumbnail.

It is blatantly inspired to the dwgbmp tool from the LibreDWG library
(the bitmap file code is mostly the same) but doesn't use the library
itself to parse the dwg file structure (which seems to be designed to
make the most difficult possible any kind of interoperability).

In fact it uses a trick simply looking for the preview signature which
was graciously provided by the OpenDesign specification for dwg files.
It also handles a case which is not documented in the specification,
i.e. preview type 6 which is simply an embedded PNG. ARES Commander 2018
files uses it, AutoCAD probably does too. What are previews types 4 and
5 I don't know.

Even if it is not the fastest way it reads the file in the fastest way
I know: it mmap the dwg and then does a search in VM.

Known limitations: first of all, it works for me for *most* of the dwg
file I have around (which is quite a lot), coming from different
sources. I tested it with R14, R2000, R2004 and R2018 files.

Preview type 3 is not implemented (that would be a WMF, never seen in
the wild). Type 2 (DIB bitmap) suffers from the same issue present in
the dwgbmp tool: it assumes a paletted 8 bit bitmap, with a palette in
the header. There are dwgs around which use a truecolor DIB so the
offset calculation in the file header is wrong (the preview contains the
full DIB data but not the bmp file header).

It could be fixed parsing the DIB header and doing the computation in
the right way.

As a thumbnail provider I only tested it with thumbler (the XFCE
thumbnail manager) on Linux x64. It *shouldn't* have endianness issues.

COMPILATION AND INSTALLATION

Standard autoconf stuff. No compilation requirement other than the
C compiler but at runtime it needs a way to convert and resize the
images.

No manpage is supplied, since the whole thing is 3 files:

- dwg.thumbnailer is the provider definition

- dwgthumb actually extracts the image from a dwg file. The two
  parameters are the source and destination file.

  The main problem is that you don't know beforehand the type of image
  contained. It simply extracts the first valid image and write it to
  the destination file.

- dwgthumbnailer is a shell script which puts all together. It has ample
  space for improvements, but it seems to work. It uses GraphicsMagick
  for the conversion and resizing steps but probably ImageMagick would
  work fine too.

After installation probably the thumbnailing service should be
restarted. In XFCE killing tumblerd does the trick, for example. 

