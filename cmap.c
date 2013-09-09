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
*   Copyright(c)1995,1996 by Jan Hubicka.See README for    *
*                     Liecnece details.                    *
*----------------------------------------------------------*
*  cmap.c cmap routines for 256color librarys-svgalib style*
***********************************************************/

#include "koules.h"
static INLINE int
col (int p, CONST float p1)
{
  p *= p1;
  if (p > 63)
    return (63);
  if (p < 0)
    return (0);
  return (p);
}

int             fadedout = 0;
void
setcustompalette (CONST int p, CONST float p1)
{
  /* 0-31 black to yellow for starwars scroller */
  /* 32-63    black to red */
  /* 64-96    for red koules */
  /* 96-128   for yellow rockets */
  /* 128-160   for green rockets */
  /* 160-192   for blue rockets */
  /* 192-256   gray cmap for stars */
  Palette         pal;
  int             i;
  for (i = 0; i < 64; i++)
    {
      int             r, g, b;
      r = g = b = 0;
      if ((i & 32) > 0)
	b = (i & 31) << 1;
      if (i < 32)
	{
	  r = (i & 3) << 4;	/* 2 bits */
	  g = (i & 4) << 3;	/* 1 bit */
	  b = (i & 24) << 1;	/* 2 bits */
	}
      pal.color[i].red = col (r + p, p1);
      pal.color[i].green = col (g + p, p1);
      pal.color[i].blue = col (b + p, p1);
    }
  for (i = 64; i < 64 + 32; i++)
    {
      int             r, g, b;
      r = g = b = 0;
      r = (32 - (i - 63)) << 1;
      if (i < 64 + 8)
	{
	  b = g = (((8 - (i - 63))) << 5) / 5;
	}
      pal.color[i].red = col (r + p, p1);
      pal.color[i].green = col (g + p, p1);
      pal.color[i].blue = col (b + p, p1);
    }
  for (i = 96; i < 96 + 32; i++)
    {
      int             r, g, b;
      r = g = b = 0;
      r = g = (32 - (i - 95)) << 1;
      if (i < 96 + 8)
	{
	  b = ((8 - (i - 95))) << 3;
	}
      pal.color[i].red = col (r + p, p1);
      pal.color[i].green = col (g + p, p1);
      pal.color[i].blue = col (b + p, p1);
    }
  for (i = 128; i < 128 + 32; i++)
    {
      int             r, g, b;
      r = g = b = 0;
      g = (32 - (i - 127)) << 1;
      if (i < 128 + 8)
	{
	  r = b = ((8 - (i - 127))) << 3;
	}
      pal.color[i].red = col (r + p, p1);
      pal.color[i].green = col (g + p, p1);
      pal.color[i].blue = col (b + p, p1);
    }
  for (i = 160; i < 160 + 32; i++)
    {
      int             r, g, b;
      r = g = b = 0;
      b = (32 - (i - 159)) << 1;
      if (i < 160 + 8)
	{
	  r = g = (((8 - (i - 159))) << 3) / 2;
	}
      pal.color[i].red = col (r + p, p1);
      pal.color[i].green = col (g + p, p1);
      pal.color[i].blue = col (b + p, p1);
    }
  for (i = 0; i < 32; i++)
    {
      pal.color[i].red = col (i * 2 + p, p1);
      pal.color[i].green = col (i * 2 + p, p1);
      pal.color[i].blue = col (p, p1);
    }
  for (i = 0; i < 32; i++)
    {
      pal.color[192 + i].red = col (i * 2 + p, p1);
      pal.color[192 + i].green = col (i * 2 + p, p1);
      pal.color[192 + i].blue = col (i * 2 + p, p1);
    }
  pal.color[0].red = 0;
  pal.color[0].green = 0;
  pal.color[0].blue = 0;
  pal.color[255].red = col (64 + p, p1);
  pal.color[255].green = col (64 + p, p1);
  pal.color[255].blue = col (64 + p, p1);
  WaitRetrace ();
  SetPalette (&pal);
}
void
fadeout ()
{
  if (!fadedout)
    {
      float           i;
      for (i = 1; i >= 0; i -= 0.1)
	{
	  setcustompalette (0, i);
	  usleep (200), tbreak = 1;

	}
      setcustompalette (-65, 0);
      fadedout = 1;
    }
}
void
fadein ()
{
  if (fadedout)
    {
      float           i;
      for (i = 0; i <= 1; i += 0.1)
	{
	  setcustompalette (0, i);
	  usleep (200), tbreak = 1;

	}
      setcustompalette (0, 1);
      fadedout = 0;
    }
}
void
fadein1 ()			/*better for star background */
{
  if (fadedout)
    {
      int             i;
      for (i = -64; i <= 0; i += 6)
	{
	  setcustompalette (i, 1);
	  usleep (200), tbreak = 1;

	}
      setcustompalette (0, 1);
      fadedout = 0;
    }
}
