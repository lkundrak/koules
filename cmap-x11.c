/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C1995 JAHUSOFT                                          *
*        Jan Hubicka                                       *
*        Dukelskych Bojovniku 1944                         *
*        390 03 Tabor                                      *
*        Czech Republic                                    *
*        Phone: 0041-361-32613                             *
*        eMail: hubicka@paru.cas.cz                        *
*----------------------------------------------------------*
*Copyright(c)1995,1996 by Jan Hubicka.See README forlicense*
*                          details.                        *
*----------------------------------------------------------*
*  cmap-x11.c colormap routines for x11                    *
*	      unclean....I know				   *
***********************************************************/

#include "koules.h"
#define col(i,y) (i>63?63:i)
#define NKEEP 28		/*how many collow keep in provate colormap */
int             fadedout = 0;
void
setmonopalette ()
{
  int             i;
  notusedc = 3;
  for (i = 0; i < 256; i++)
    pixels[i] = BlackPixel (dp, screen),
      spixels[i] = 0;
  for (i = 0; i < 16; i++)
    {
      pixels[i] = WhitePixel (dp, screen);
      spixels[i] = 64;
      pixels[i + 16] = WhitePixel (dp, screen);
      spixels[i + 16] = 64;
      pixels[i + 32 + 16] = WhitePixel (dp, screen);
      spixels[i + 32 + 16] = 64;
      pixels[i + 192 + 16] = WhitePixel (dp, screen);
      spixels[i + 192 + 16] = 64;
      pixels[i + 64] = WhitePixel (dp, screen);
      spixels[i + 64] = 64;
      pixels[i + 96] = WhitePixel (dp, screen);
      spixels[i + 96] = 64;
      pixels[i + 128] = WhitePixel (dp, screen);
      spixels[i + 128] = 64;
      pixels[i + 160] = WhitePixel (dp, screen);
      spixels[i + 160] = 64;
    }
  pixels[0] = BlackPixel (dp, screen),
    pixels[255] = WhitePixel (dp, screen),
    spixels[0] = 0;
}
static int
setcustompalette1 (CONST int ncolors)
{
  /* 0-31 black to yellow for starwars scroller */
  /* 32-63    black to red */
  /* 64-96    for red koules */
  /* 96-128   for yellow rockets */
  /* 128-160   for green rockets */
  /* 160-192   for blue rockets */
  /* 192-256   gray cmap for stars */
  unsigned long   pixels1[256];
  int             npixels1;
  unsigned char   pal[256][3];
  int             i;
  int             y;
  const int       p = 0;
  int             lr = -1, lg = -1, lb = -1, lpixel = -1;
  XColor          c[256];
  int             nallocated = 0;
  if (useprivate)
    {
      nallocated = NKEEP;
      for (i = 0; i < NKEEP; i++)
	c[i].pixel = i;
      XQueryColors (dp, DefaultColormap (dp, screen), c, NKEEP);
      for (i = 0; i < NKEEP; i++)
	XStoreColors (dp, colormap, c, NKEEP);
    }
  for (i = 0; i < 64; i++)
    {
      int             r, g, b;
      r = g = b = 0;
      y = (i * ncolors / 32) * 32 / ncolors;
      if ((y & 32) > 0)
	b = (y & 31) << 1;
      if (y < 32)
	{
	  r = (y & 3) << 4;	/* 2 bits */
	  g = (y & 4) << 3;	/* 1 bit */
	  b = (y & 24) << 1;	/* 2 bits */
	}
      pal[i][0] = col (r + p, p1);
      pal[i][1] = col (g + p, p1);
      pal[i][2] = col (b + p, p1);
    }
  for (i = 64; i < 64 + 32; i++)
    {
      int             r, g, b;
      y = (i * ncolors / 32) * 32 / ncolors;
      if (y < 64)
	y = 64;
      r = g = b = 0;
      r = (32 - (y - 63)) << 1;
      if (i < 64 + 8)
	{
	  b = g = (((8 - (y - 63))) << 5) / 5;
	}
      pal[i][0] = col (r + p, p1);
      pal[i][1] = col (g + p, p1);
      pal[i][2] = col (b + p, p1);
    }
  for (i = 96; i < 96 + 32; i++)
    {
      int             r, g, b;
      y = (i * ncolors / 32) * 32 / ncolors;
      if (y < 96)
	y = 96;
      r = g = b = 0;
      r = g = (32 - (y - 95)) << 1;
      if (i < 96 + 8)
	{
	  b = ((8 - (y - 95))) << 3;
	}
      pal[i][0] = col (r + p, p1);
      pal[i][1] = col (g + p, p1);
      pal[i][2] = col (b + p, p1);
    }
  for (i = 128; i < 128 + 32; i++)
    {
      int             r, g, b;
      y = (i * ncolors / 32) * 32 / ncolors;
      if (y < 128)
	y = 128;
      r = g = b = 0;
      g = (32 - (y - 127)) << 1;
      if (i < 128 + 8)
	{
	  r = b = ((8 - (i - 127))) << 3;
	}
      pal[i][0] = col (r + p, p1);
      pal[i][1] = col (g + p, p1);
      pal[i][2] = col (b + p, p1);
    }
  for (i = 160; i < 160 + 32; i++)
    {
      int             r, g, b;
      y = (i * ncolors / 32) * 32 / ncolors;
      if (y < 160)
	y = 160;
      r = g = b = 0;
      b = (32 - (y - 159)) << 1;
      if (i < 160 + 8)
	{
	  r = g = (((8 - (y - 159))) << 3) / 2;
	}
      pal[i][0] = col (r + p, p1);
      pal[i][1] = col (g + p, p1);
      pal[i][2] = col (b + p, p1);
    }
  for (i = 0; i < 32; i++)
    {
      y = (i * ncolors / 32) * 32 / ncolors;
      if (ncolors < 8)
	{
	  pal[i][0] = 64;
	  pal[i][1] = 64;
	  pal[i][2] = 0;
	}
      else
	{
	  pal[i][0] = col (y * 2 + p, p1);
	  pal[i][1] = col (y * 2 + p, p1);
	  pal[i][2] = col (p, p1);
	}
    }

  for (i = 0; i < 32; i++)
    {
      y = (i * ncolors / 32) * 32 / ncolors;
      if (ncolors < 8)
	{
	  pal[192 + i][0] = 64;
	  pal[192 + i][1] = 64;
	  pal[192 + i][2] = 64;
	}
      else
	{
	  pal[192 + i][0] = col (y * 2 + p, p1);
	  pal[192 + i][1] = col (y * 2 + p, p1);
	  pal[192 + i][2] = col (y * 2 + p, p1);
	}
    }
  pal[0][0] = 0;
  pal[0][1] = 0;
  pal[0][2] = 0;
  pal[255][0] = col (63 + p, p1);
  pal[255][1] = col (63 + p, p1);
  pal[255][2] = col (63 + p, p1);
  npixels1 = 0;
  for (i = 0; i < 256; i++)
    {
      float           sat;
      sat = pal[i][0] * 0.3 + pal[i][1] * 0.5 + pal[i][2] * 0.2;
      if (monochrome)
	sat = pow (sat / 64, 0.6) * 64;
      spixels[i] = sat;
      if (ncolors == 32)
	{
	  opixels[i] = sat;
	}
    }
  for (i = 0; i < 256 && !blackwhite; i++)
    {
      if (lr == pal[i][0] && lg == pal[i][1] && lb == pal[i][2])
	pixels[i] = lpixel;
      else
	{
	  XColor          color;
	  if (!monochrome)
	    {
	      color.flags = DoRed | DoGreen | DoBlue;
	      color.red = (unsigned short) (pal[i][0]) * 256 * 4;
	      color.green = (unsigned short) (pal[i][1]) * 256 * 4;
	      color.blue = (unsigned short) (pal[i][2]) * 256 * 4;
	      color.pixel = npixels1;
	    }
	  else
	    {
	      color.flags = DoRed | DoGreen | DoBlue;
	      color.red = (unsigned short) (spixels[i]) * 256 * 4;
	      color.green = (unsigned short) (spixels[i]) * 256 * 4;
	      color.blue = (unsigned short) (spixels[i]) * 256 * 4;
	      color.pixel = npixels1;
	    }
	  if (!useprivate && !XAllocColor (dp, colormap, &color))
	    {
	      XFreeColors (dp, colormap, pixels1, npixels1, 0L);
	      return (0);
	    }
	  if (useprivate)
	    {
	      int             i;
	      for (i = 0; i < nallocated; i++)
		{
		  if ((c[i].red & 0xfc00) == (color.red && 0xfc00) && (c[i].blue & 0xfc00) == (color.blue & 0xfc00) &&
		      (c[i].green & 0xfc00) == (color.green & 0xfc00))
		    {
		      color.pixel = i;
		      break;
		    }
		}
	      if (i == nallocated)
		color.pixel = i, nallocated++;
	      if (nallocated >= 256)
		printf ("Cmap owerflow!");
	      XStoreColor (dp, colormap, &color);
	      c[i].red = color.red;
	      c[i].green = color.green;
	      c[i].blue = color.blue;
	    }
	  pixels1[npixels1] = color.pixel;
	  lpixel = color.pixel;
	  lr = pal[i][0];
	  lg = pal[i][1];
	  lb = pal[i][2];
	  npixels1++;

	}
      pixels[i] = lpixel;
    }
  return (1);
}
void
setcustompalette (CONST int p, CONST float p1)
{
  int             ncolor, i;
  if (blackwhite)
    {
      nofade = 1;
      monochrome = 1;
      setcustompalette1 (32);
      setmonopalette ();
      return;
    }
  printf ("Allocating pallette\n");
  for (ncolor = 32; ncolor > 8; ncolor -= 4)
    {
      printf ("%i-", ncolor);
      if (setcustompalette1 (ncolor))
	{
	  notusedc = 0;
	  for (i = 0; i < 256; i++)
	    if (notusedc == pixels[i])
	      notusedc++, i = 0;
	  printf ("ok\n");
	  return;
	}
    }
  printf ("failed...using slow algoritmus and private colormap\n");
  colormap = XCreateColormap (dp, wi,
			      DefaultVisual (dp, XDefaultScreen (dp)),
			      AllocAll);
  XSetWindowColormap (dp, wi, colormap);
  useprivate = 1;

  for (ncolor = 32; ncolor > 0; ncolor -= 1)
    {
      printf ("%i-", ncolor);
      if (setcustompalette1 (ncolor))
	{
	  notusedc = 0;
	  for (i = 0; i < 256; i++)
	    if (notusedc == pixels[i])
	      notusedc++, i = 0;
	  printf ("ok\n");
	  return;
	}
    }
  printf ("Could not allocate colormap try -m option\n");
  exit (-1);
}
/* adapted from 'xscreensaver',
 */
#define XFADE_IN 1
#define XFADE_OUT 0
#define NUM_COLORS 256
void
Fade (CONST int steps)
{
  static int      state = XFADE_IN;
  static XColor   orig_colors[NUM_COLORS];
  XColor          curr_colors[NUM_COLORS];
  int             i, j;
  static Colormap *fade_cmap, new_cmap;
  if (!fadeenable || nofade)
    return;
  nopause = 1;
  /* Set the requested pixels */
  for (i = 0; i < NUM_COLORS; ++i)
    orig_colors[i].pixel = i;

  state = ((state == XFADE_IN) ? XFADE_OUT : XFADE_IN);
  /* Make a copy of the default colormap and use that in fade */
  if (useprivate)
    {
      if (state == XFADE_OUT)
	{
	  XQueryColors (dp, colormap, orig_colors,
			NUM_COLORS);
	  fade_cmap = &colormap;
	}
    }
  else
    {
      XQueryColors (dp, colormap, orig_colors, NUM_COLORS);
      if (state == XFADE_OUT)
	{
	  new_cmap = XCreateColormap (dp,
				      DefaultRootWindow (dp),
				    DefaultVisual (dp, XDefaultScreen (dp)),
				      AllocAll);
	  XStoreColors (dp, new_cmap, orig_colors, NUM_COLORS);
	  XGrabServer (dp);
	  XInstallColormap (dp, new_cmap);
	  fade_cmap = &new_cmap;
	}
    }
  XSync (dp, False);

  memcpy (curr_colors, orig_colors, NUM_COLORS * sizeof (XColor));
  if (state == XFADE_OUT)
    {
      for (i = steps; i > 0; i--)
	{
	  for (j = 0; j < NUM_COLORS; j++)
	    {
	      curr_colors[j].red =
		orig_colors[j].red * i / steps;
	      curr_colors[j].green =
		orig_colors[j].green * i / steps;
	      curr_colors[j].blue =
		orig_colors[j].blue * i / steps;
	    }
	  XStoreColors (dp, *fade_cmap,
			curr_colors, NUM_COLORS);
	  usleep (10);
	  XSync (dp, False);
	}
    }
  else
    {
      for (i = 0; i < steps; i++)
	{
	  for (j = 0; j < NUM_COLORS; j++)
	    {
	      curr_colors[j].red =
		orig_colors[j].red * i / steps;
	      curr_colors[j].green =
		orig_colors[j].green * i / steps;
	      curr_colors[j].blue =
		orig_colors[j].blue * i / steps;
	    }
	  XStoreColors (dp, *fade_cmap,
			curr_colors, NUM_COLORS);
	  usleep (10);
	  XSync (dp, False);
	}
    }

  if (state == XFADE_IN)
    {
      /* Restore the original colormap */
      if (useprivate)
	{
	  XStoreColors (dp, colormap,
			orig_colors, NUM_COLORS);
	}
      else
	{
	  XInstallColormap (dp, colormap);
	  XUngrabServer (dp);
	  XFreeColormap (dp, *fade_cmap);
	}
      XSync (dp, False);
    }
  nopause = 0;
}
void
fadeout ()
{
  if (!fadedout)
    {
      Fade (30);
      fadedout = 1;
      tbreak = 1;
    }
}
void
fadein ()
{
  if (fadedout)
    {
      Fade (30);
      fadedout = 0;
      tbreak = 1;
    }
}
void
fadein1 ()			/*better for star background */
{
  if (fadedout)
    {
      Fade (30);
      fadedout = 0;
      tbreak = 1;
    }
}
