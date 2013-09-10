/* Addapted from: */
/* Framebuffer Graphics Libary for Linux, Copyright 1993 Harm Hanemaayer */
/* text.c       Text writing and fonts */

#ifdef MITSHM
#include <stdlib.h>
#include <string.h>
#include "inlstring.h"		/* include inline string operations */
#include "interface.h"


typedef unsigned char uchar;
#define ASSIGNVP8(x, y, vp) vp = (unsigned char *)VScreenToBuffer(current) + (y)*MAPWIDTH + (x);
static void
putbox (int x, int y, int w, int h, void *b)
{
  uchar          *vp;		/* screen pointer */
  uchar          *bp;		/* bitmap pointer */
  int             i;
  ASSIGNVP8 (x, y, vp);
  bp = b;
  for (i = 0; i < h; i++)
    {
      __memcpy (vp, bp, w);
      /*bp += bw; */
      vp += MAPWIDTH;
    }
}


static void
putboxmask (int x, int y, int w, int h, void *b)
{
  uchar          *bp = b;
  uchar          *vp;
  int             i;
  ASSIGNVP8 (x, y, vp);
  for (i = 0; i < h; i++)
    {
      uchar          *endoflinebp = bp + w;
#ifdef __i386__
      while (bp < endoflinebp - 3)
	{
	  unsigned int    c4 = *(unsigned int *) bp;
	  if ((c4 & 0xff) != notusedc)
	    *vp = (uchar) c4;
	  c4 >>= 8;
	  if ((c4 & 0xff) != notusedc)
	    *(vp + 1) = (uchar) c4;
	  c4 >>= 8;
	  if ((c4 & 0xff) != notusedc)
	    *(vp + 2) = (uchar) c4;
	  c4 >>= 8;
	  if ((c4 & 0xff) != notusedc)
	    *(vp + 3) = (uchar) c4;
	  bp += 4;
	  vp += 4;
	}
#endif
      while (bp < endoflinebp)
	{
	  uchar           c = *bp;
	  if (c != notusedc)
	    *vp = c;
	  bp++;
	  vp++;
	}
      vp += MAPWIDTH - w;
    }
}



/* Text/font functions */

static int      font_width = 8;
static int      font_height = 8;
static char    *font_address;
static int      font_charactersize = 64;
static int      font_writemode = WRITEMODE_OVERWRITE;



void
SetFont (int fw, int fh, void *font)
{
  font_width = fw;
  font_height = fh;
  font_charactersize = font_width * font_height;
  font_address = font;
}

void
SetWriteMode (int m)
{
  font_writemode = m;
}


void
FWrite (int x, int y, char *s)
{
/* clipping in putbox */
  int             i;
  i = 0;
  if (!(font_writemode & WRITEMODE_MASKED))
    {
      while (s[i] != 0)
	{
	  putbox (x + i * font_width, y, font_width,
		  font_height, font_address +
		  (unsigned char) s[i] * font_charactersize);
	  i++;
	}
    }
  else
    {				/* masked write */
      while (s[i] != 0)
	{
	  putboxmask (x + i * font_width, y, font_width,
		      font_height, font_address +
		      (unsigned char) s[i] * font_charactersize);
	  i++;
	}
    }
}

void
ExpandFont (int fw, int fh, int fg, void *_f1, void *_f2)
{
/* Convert bit-per-pixel font to byte(s)-per-pixel font */
  uchar          *f1 = _f1;
  uchar          *f2 = _f2;
  int             i, x, y, b = 0;	/* keep gcc happy with b = 0 - MW */

  for (i = 0; i < 256; i++)
    {
      for (y = 0; y < fh; y++)
	for (x = 0; x < fw; x++)
	  {
	    if (x % 8 == 0)
	      b = *f1++;
	    if (b & (128 >> (x % 8)))
	      {			/* pixel */
		*f2 = fg;
		f2++;
	      }
	    else
	      {			/* no pixel */
		*f2 = notusedc;
		f2++;
	      }
	  }
    }
}


void
DrawBlackMaskedText (int xp, int yp, char *text)
{
  if (!shm)
    {
      XDrawString (dp, current.pixmap, blackgc, xp, yp + 7, text, strlen (text));
    }
  else
    {
      SetWriteMode (WRITEMODE_MASKED);
      SetFont (8, 8, fontblack);
      FWrite (xp, yp, text);

    }
}
void
DrawWhiteMaskedText (int xp, int yp, char *text)
{
  if (!shm)
    {
      XDrawString (dp, current.pixmap, whitegc, xp, yp + 7, text, strlen (text));
    }
  else
    {
      SetWriteMode (WRITEMODE_MASKED);
      SetFont (8, 8, fontwhite);
      FWrite (xp, yp, text);

    }
}
void
DrawText (int xp, int yp, char *text)
{
  if (!shm)
    {
      XDrawString (dp, current.pixmap, whitegc, xp, yp + 7, text, strlen (text));
    }
  else
    {
      SetWriteMode (WRITEMODE_MASKED);
      SetFont (8, 8, fontwhite);
      FWrite (xp, yp, text);

    }
}
#endif
