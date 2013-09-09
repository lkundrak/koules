/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C1995 JAHUSOFT                                          *
*        Jan Hubicka                                       *
*        Dukelskych Bojovniku 1944                         *
*        390 03 Tabor                                      *
*        Czech Republic                                    *
*        Phone: 0041-361-32613                             *
*        eMail: hubicka@limax.paru.cas.cz                  *
*----------------------------------------------------------*
*   Copyright(c)1995,1996 by Jan Hubicka.See README for    *
*                    licence details.                      *
*----------------------------------------------------------*
*  framebuffer.c fast 8 bit framebuffer bitmap creation    *
*                routines                                  *
***********************************************************/
#include "koules.h"


#define NCOLORS 32

#define HOLE_XCENTER (2*HOLE_RADIUS-3*HOLE_RADIUS/4)
#define HOLE_YCENTER (2*HOLE_RADIUS-HOLE_RADIUS/4)
#define HOLE_MAX_RADIUS (HOLE_RADIUS/DIV+0.5*HOLE_RADIUS/DIV)
#define HOLE_SIZE_MAX (radius*radius)
#ifndef NODIRECT
/*
 * hardcoded bitmap drawing routines 
 */
static char    *
draw_ball_bitmap (int radius, CONST int color)
{
  char           *bitmap = NULL, *point;
  int             x, y, r;
  radius /= DIV;
  if ((bitmap = alloca ((radius * 2) * (radius * 2) + 2)) == NULL)
    perror ("create_ball_bitmap"), exit (-1);
  point = bitmap;
  for (y = 0; y < radius * 2; y++)
    for (x = 0; x < radius * 2; x++, point++)
      {
	if ((x - radius) * (x - radius) + (y - radius) * (y - radius)
	    < (radius - 0.5) * (radius - 0.5))
	  {
	    r = (x - 3 * radius / 4) * (x - 3 * radius / 4) +
	      (y - radius / 4) * (y - radius / 4);
	    r = r * 32 / (1.5 * radius) / (1.5 * radius);
	    if (r > 31)
	      r = 31;
	    *point = color + r;
	  }
	else
	  *point = 0;
      }
  return (CompileBitmap (radius * 2, radius * 2, (char *) bitmap));
}
static char    *
draw_reversed_ball_bitmap (int radius, CONST int color)
{
  char           *bitmap = NULL, *point;
  int             x, y, r;
  radius /= DIV;
  if ((bitmap = alloca ((radius * 2) * (radius * 2) + 2)) == NULL)
    perror ("create_ball_bitmap"), exit (-1);
  point = bitmap;
  for (y = 0; y < radius * 2; y++)
    for (x = 0; x < radius * 2; x++, point++)
      {
	if ((x - radius) * (x - radius) + (y - radius) * (y - radius)
	    < (radius - 0.5) * (radius - 0.5))
	  {
	    r = (x - 3 * radius / 4) * (x - 3 * radius / 4) +
	      (y - radius / 4) * (y - radius / 4);
	    r = r * 32 / (1.5 * radius) / (1.5 * radius);
	    if (r > 31)
	      r = 31;
	    *point = color + 16 + r / 2;
	  }
	else
	  *point = 0;
      }
  return (CompileBitmap (radius * 2, radius * 2, (char *) bitmap));
}
static char    *
draw_apple_bitmap (int radius, CONST int color)
{
  char           *bitmap = NULL, *point;
  int             x, y, r;
  int             radius1;
  radius /= DIV;
  if ((bitmap = alloca ((radius * 2) * (radius * 2) + 2)) == NULL)
    perror ("create_ball_bitmap"), exit (-1);
  point = bitmap;
  for (y = 0; y < radius * 2; y++)
    for (x = 0; x < radius * 2; x++, point++)
      {
	if (DIV == 2)
	  radius1 = radius * (abs (x - radius) / 2 + 25) / 30;
	else
	  radius1 = radius * (abs (x - radius) / 2 + 50) / 60;
	if (radius1 > radius)
	  radius1 = radius;
	if ((x - radius) * (x - radius) + (y - radius) * (y - radius)
	    < ((radius1) * (radius1)))
	  {
	    r = (x - 3 * radius / 4) * (x - 3 * radius / 4) +
	      (y - radius / 4) * (y - radius / 4);
	    r = 3 + r * 22 / (1.5 * radius1) / (1.5 * radius1);
	    if (r > 31)
	      r = 31;
	    *point = color + r;
	  }
	else
	  *point = 0;
      }
  return (CompileBitmap (radius * 2, radius * 2, (char *) bitmap));
}
#else
static float    err = 0.0;
#ifdef XSUPPORT
static INLINE int
errdist (int c)
{
  float           sat, merr;
  int             i;
  int             p;
  c &= 0xff;
  sat = opixels[c] + err;
  merr = sat - spixels[c];
  p = c;
  if (sat < spixels[c])
    {
      int             max = c / 32;
      max = (max + 1) * 32;
      for (i = c; i < max && i < 255; i++)
	{
	  if (fabs (merr) > fabs (spixels[i] - sat))
	    {
	      p = i;
	      merr = sat - spixels[i];
	    }
	}
    }
  if (sat > spixels[c])
    {
      int             min = c / 32;
      min *= 32;
      for (i = c; i > min && i > 0; i--)
	{
	  if (fabs (merr) > fabs (spixels[i] - sat))
	    {
	      p = i;
	      merr = sat - spixels[i];
	    }
	}
    }
  if (fabs (merr) > fabs (64 - sat))
    {
      p = 255;
      merr = sat - 64;
    }
  if (fabs (merr) > fabs (sat))
    {
      p = 32;
      merr = sat;
    }
  err = merr;
  return (p);
}
#else
#define errdist(c) c
#endif
static          BitmapType
draw_ball_bitmap (int radius, CONST int color)
{
  RawBitmapType   bbitmap;
  int             x, y, r, c;
  radius /= DIV;
  err = 0.0;
  bbitmap = CreateBitmap ((radius * 2), (radius * 2));
  for (y = 0; y < radius * 2; y++)
    for (x = 0; x < radius * 2; x++)
      {
	if ((x - radius) * (x - radius) + (y - radius) * (y - radius)
	    < (radius - 0.5) * (radius - 0.5))
	  {
	    r = (x - 3 * radius / 4) * (x - 3 * radius / 4) +
	      (y - radius / 4) * (y - radius / 4);
	    r = r * 32 / (1.5 * radius) / (1.5 * radius);
	    if (r > 31)
	      r = 31;
	    c = errdist (color + r);
	    BSetPixel (bbitmap, x, y, c);
	  }
	else
	  {
	    err = 0.0;
	    BSetPixel (bbitmap, x, y, 0);
	  }
      }
  return (CompileBitmap (radius * 2, radius * 2, bbitmap));
}
static          BitmapType
draw_reversed_ball_bitmap (int radius, CONST int color)
{
  RawBitmapType   bbitmap;
  int             x, y, r, c;
  radius /= DIV;
  err = 0.0;
  bbitmap = CreateBitmap ((radius * 2), (radius * 2));
  for (y = 0; y < radius * 2; y++)
    for (x = 0; x < radius * 2; x++)
      {
	if ((x - radius) * (x - radius) + (y - radius) * (y - radius)
	    < (radius - 0.5) * (radius - 0.5))
	  {
	    r = (x - 3 * radius / 4) * (x - 3 * radius / 4) +
	      (y - radius / 4) * (y - radius / 4);
	    r = r * 32 / (1.5 * radius) / (1.5 * radius);
	    if (r > 31)
	      r = 31;
	    c = errdist (color + 16 + r / 2);
	    BSetPixel (bbitmap, x, y, c);
	  }
	else
	  {
	    err = 0.0;
	    BSetPixel (bbitmap, x, y, 0);
	  }
      }
  return (CompileBitmap (radius * 2, radius * 2, bbitmap));
}
static          BitmapType
draw_apple_bitmap (int radius, CONST int color)
{
  RawBitmapType   bitmap;
  int             x, y, r, c;
  int             radius1;
  radius /= DIV;
  bitmap = CreateBitmap ((radius * 2), (radius * 2));
  err = 0;
  for (y = 0; y < radius * 2; y++)
    for (x = 0; x < radius * 2; x++)
      {
	if (DIV == 2)
	  radius1 = radius * (abs (x - radius) / 2 + 25) / 30;
	else
	  radius1 = radius * (abs (x - radius) / 2 + 50) / 60;
	if (radius1 > radius)
	  radius1 = radius;
	if ((x - radius) * (x - radius) + (y - radius) * (y - radius)
	    < ((radius1) * (radius1)))
	  {
	    r = (x - 3 * radius / 4) * (x - 3 * radius / 4) +
	      (y - radius / 4) * (y - radius / 4);
	    r = 3 + r * 22 / (1.5 * radius1) / (1.5 * radius1);
	    if (r > 31)
	      r = 31;
	    c = errdist (color + r);
	    BSetPixel (bitmap, x, y, c);
	  }
	else
	  {
	    BSetPixel (bitmap, x, y, 0);
	  }
      }
  return (CompileBitmap (radius * 2, radius * 2, bitmap));
}
#endif
void
create_bitmap ()
{
  int             x, y, r, po, radius;
#ifndef NODIRECT
  char            hole_data[HOLE_RADIUS * 2][HOLE_RADIUS * 2];
  char            ehole_data[HOLE_RADIUS * 2][HOLE_RADIUS * 2];
#else
  int             c;
  RawBitmapType   hole_data, ehole_data;
#endif
  printf ("creating bitmaps...\n");
#ifndef NODIRECT
  for (x = 0; x < HOLE_RADIUS * 2; x++)
    for (y = 0; y < HOLE_RADIUS * 2; y++)
      {
	if (DIV == 1)
	  radius = HOLE_RADIUS / 2 + (int) (atan (fabs (x - HOLE_RADIUS + 0.5) / fabs (y - HOLE_RADIUS + 0.5)) * HOLE_RADIUS / 2) % (HOLE_RADIUS / 2);
	else
	  radius = HOLE_RADIUS / 4;
	if ((x - HOLE_RADIUS / DIV) * (x - HOLE_RADIUS / DIV) + (y - HOLE_RADIUS / DIV) * (y - HOLE_RADIUS / DIV)
	    < radius * radius)
	  {
	    r = (x - HOLE_RADIUS / DIV) * (x - HOLE_RADIUS / DIV) +
	      (y - HOLE_RADIUS / DIV) * (y - HOLE_RADIUS / DIV);
	    r = r * 24 / HOLE_SIZE_MAX;
	    if (r > 23)
	      r = 23;
	    hole_data[x][y] = 64 + r + 1;
	    ehole_data[x][y] = 128 + r + 1;
	  }
	else
	  hole_data[x][y] = 0,
	    ehole_data[x][y] = 0;
      }
  hole_bitmap = (CompileBitmap (HOLE_RADIUS * 2, HOLE_RADIUS * 2, (char *) hole_data));
  ehole_bitmap = (CompileBitmap (HOLE_RADIUS * 2, HOLE_RADIUS * 2, (char *) ehole_data));
#else
  ehole_data = CreateBitmap ((HOLE_RADIUS * 2), (HOLE_RADIUS * 2));
  hole_data = CreateBitmap ((HOLE_RADIUS * 2), (HOLE_RADIUS * 2));
  err = 0;
  for (x = 0; x < HOLE_RADIUS * 2; x++)
    for (y = 0; y < HOLE_RADIUS * 2; y++)
      {
	if (DIV == 1)
	  radius = HOLE_RADIUS / 2 + (int) (atan (fabs (x - HOLE_RADIUS + 0.5) / fabs (y - HOLE_RADIUS + 0.5)) * HOLE_RADIUS / 2) % (HOLE_RADIUS / 2);
	else
	  radius = HOLE_RADIUS / 4;
	if ((x - HOLE_RADIUS / DIV) * (x - HOLE_RADIUS / DIV) + (y - HOLE_RADIUS / DIV) * (y - HOLE_RADIUS / DIV)
	    < radius * radius)
	  {
	    r = (x - HOLE_RADIUS / DIV) * (x - HOLE_RADIUS / DIV) +
	      (y - HOLE_RADIUS / DIV) * (y - HOLE_RADIUS / DIV);
	    r = r * 24 / HOLE_SIZE_MAX;
	    if (r > 23)
	      r = 23;
	    c = errdist (64 + r + 1);
	    BSetPixel (hole_data, x, y, c);
	    c = errdist (128 + r + 1);
	    BSetPixel (ehole_data, x, y, c);
	  }
	else
	  {
	    BSetPixel (hole_data, x, y, 0);
	    BSetPixel (ehole_data, x, y, 0);
	  }
      }
  ehole_bitmap = (CompileBitmap ((HOLE_RADIUS * 2), (HOLE_RADIUS * 2), ehole_data));
  hole_bitmap = (CompileBitmap (HOLE_RADIUS * 2, HOLE_RADIUS * 2, hole_data));
#endif

  for (po = 0; po < MAXROCKETS; po++)
    {
      eye_bitmap[po] = draw_ball_bitmap (EYE_RADIUS, 32 + 32 * po);
    }
  ball_bitmap = draw_ball_bitmap (BALL_RADIUS, ball (0));
  mouse_bitmap = draw_ball_bitmap (MOUSE_RADIUS * DIV, 32 + 32 * 2);
  bball_bitmap = draw_ball_bitmap (BBALL_RADIUS, 4 * 32);
  apple_bitmap = draw_apple_bitmap (APPLE_RADIUS, ball (0));
  inspector_bitmap = draw_ball_bitmap (INSPECTOR_RADIUS, 160);
  lunatic_bitmap = draw_reversed_ball_bitmap (LUNATIC_RADIUS, 192);
  lball_bitmap[0] = draw_ball_bitmap (BALL_RADIUS, 4 * 32);
  lball_bitmap[1] = draw_ball_bitmap (BALL_RADIUS, 5 * 32);
  lball_bitmap[2] = draw_reversed_ball_bitmap (BALL_RADIUS, 192);
  lball_bitmap[3] = draw_reversed_ball_bitmap (BALL_RADIUS, 0);
  lball_bitmap[4] = draw_reversed_ball_bitmap (BALL_RADIUS, 3 * 32 - 5);
 /* lball_bitmap[5] = draw_reversed_ball_bitmap (BALL_RADIUS, 4 * 32 - 5);*/

  for (x = 0; x < 5; x++)
    rocket_bitmap[x] = draw_ball_bitmap (ROCKET_RADIUS, rocketcolor[x]);



}









#ifndef XSUPPORT
static void
createbackground ()
{
/* Create fancy dark red background */
  int             x, y;
#ifndef NODIRECT
  char           *pixel = background->vbuf;
#endif
  for (y = 0; y < MAPHEIGHT + 20; y++)
    for (x = 0; x < MAPWIDTH; x++)
      {
	int             i = 0;
	int             n = 0;
	int             c;
	if (x > 0)
	  {
#ifndef NODIRECT
	    i += *(pixel - 1) - back (0);
#else
	    i += SGetPixel (x - 1, y);
#endif
	    n++;
	  }
	if (y > 0)
	  {
#ifndef NODIRECT
	    i += *(pixel - MAPWIDTH) - back (0);
#else
	    i += SGetPixel (x, y - 1);
#endif
	    n++;
	  }
	c = (i + (rand () % 16)) / (n + 1);
	if (c > 9)
	  c = 9;
#ifndef NODIRECT
	*pixel = back (0) + c;
	pixel++;
#else
	SPutPixel (x, y, back (0) + c);
#endif
      }
}
#else
#ifdef MITSHM
static void
Shmcreatebackground ()
{
/* Create fancy dark red background */
  int             x, y;
  unsigned char  *pixel = (unsigned char *) background.vbuff;
  for (y = 0; y < MAPHEIGHT + 20; y++)
    for (x = 0; x < MAPWIDTH; x++)
      {
	int             i = 0;
	int             n = 0;
	int             c;
	if (x > 0)
	  {
	    i += *(pixel - 1) - back (0);
	    n++;
	  }
	if (y > 0)
	  {
	    i += *(pixel - MAPWIDTH) - back (0);
	    n++;
	  }
	c = (i + (rand () % 16)) / (n + 1);
	if (c > 9)
	  c = 9;
	*pixel = back (0) + c;
	pixel++;
      }
  pixel = (unsigned char *) background.vbuff;
  for (y = 0; y < MAPHEIGHT + 20; y++)
    for (x = 0; x < MAPWIDTH; x++)
      *pixel = (unsigned char) pixels[*pixel],
	pixel++;
}
#endif
static void
createbackground ()
{
/* Create fancy dark red background */
  int             x, y;
  XImage         *img;
  char           *data;
#ifdef MITSHM
  if (shm)
    {
      Shmcreatebackground ();
      return;
    }
#endif
  if ((data = (char *) calloc ((MAPWIDTH + BitmapPad (dp)) * (MAPHEIGHT + 20) * 4, 1)) == NULL)
    perror ("Memory Error"), exit (2);
  img = XCreateImage (dp, DefaultVisual (dp, screen), DefaultDepth (dp, screen),
		      ZPixmap, 0, data, MAPWIDTH, MAPHEIGHT + 20,
		      BitmapPad (dp), 0);
  for (y = 0; y < MAPHEIGHT + 20; y++)
    for (x = 0; x < MAPWIDTH; x++)
      {
	int             i = 0;
	int             n = 0;
	int             c;
	if (x > 0)
	  {
	    i += XGetPixel (img, x - 1, y);
	    n++;
	  }
	if (y > 0)
	  {
	    i += XGetPixel (img, x, y - 1);
	    n++;
	  }
	c = (i + (rand () % 16)) / (n + 1);
	if (c > 9)
	  c = 9;
	/*gl_setpixel (x, y, back (0) + c); */
	XPutPixel (img, x, y, c & 0xff);
      }
  for (y = 0; y < MAPHEIGHT + 20; y++)
    for (x = 0; x < MAPWIDTH; x++)
      XPutPixel (img, x, y, pixels[back (0) + XGetPixel (img, x, y)]);
  XPutImage (dp, current.pixmap, gc, img, 0, 0, 0, 0, MAPWIDTH, MAPHEIGHT + 20);
  XSync (dp, 1);
  XDestroyImage (img);
}
#endif
void
drawstarbackground ()
{
/* Create fancy dark red background */
  int             x;
  int             x1, y1, c1;
  SetScreen (starbackground);
  ClearScreen ();
  for (x = 0; x < 700 / DIV / DIV; x++)
    {
      x1 = rand () % MAPWIDTH;
      y1 = rand () % (MAPHEIGHT + 20);
      c1 = rand () % 32 + 192;
      SSetPixel (x1, y1, c1);
    }
}




void
drawbackground ()
{				/*int i; */
/* Build up background from map data */
  SetScreen (background);
  ClearScreen ();
  createbackground ();
  EnableClipping ();
#ifdef MITSHM
  if (shm)
    HLine (0, MAPHEIGHT, MAPWIDTH - 1, back (16));
  else
#endif
    Line (0, MAPHEIGHT, MAPWIDTH - 1, MAPHEIGHT, back (16));
  DisableClipping ();
}
