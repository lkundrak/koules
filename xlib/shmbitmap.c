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
*                     licence details.                     *
*----------------------------------------------------------*
*  shmbitmap.c framebuffer graphics routines               *
***********************************************************/
#ifdef MITSHM
/*adapted from: */
/* Framebuffer Graphics Libary for Linux, Copyright 1993 Harm Hanemaayer */
/* cbitmap.c    Compiled bitmaps */
/*thanks */


#include <stdlib.h>
#include "../koules.h"
#define BYTEWIDTH MAPWIDTH
#define __clipx1 0
#define __clipx2 (MAPWIDTH-2)
#define __clipy1 0
#define __clipy2 (MAPHEIGHT+17)
#define VBUF current.vbuff



typedef unsigned char uchar;

void
ShmCompileBitmap (int w, int h, void *_dp1, void *_dp2)
{
/* Compiled format: <bytes_to_skip (0-254)><number_of_pixels (0-255)> */
/*                  <pixel_data>[<end_of_line(255)>]... */
  uchar          *dp1 = _dp1;
  uchar          *dp2 = _dp2;
  int             i;
  for (i = 0; i < h; i++)
    {
      int             x = 0;
      while (x < w)
	{
	  int             count;
	  /* count zeroes */
	  count = 0;
	  while (x < w && *(dp1 + count) == notusedc && count < 254)
	    {
	      count++;
	      x++;
	    }
	  dp1 += count;
	  if (x < w)
	    {
	      *dp2++ = count;
	      /* count nonzeroes */
	      count = 0;
	      while (x < w && *(dp1 + count) != notusedc && count < 255)
		{
		  *(dp2 + count + 1) = *(dp1 + count);
		  count++;
		  x++;
		}
	      *dp2 = count;
	      dp2 += count + 1;
	      dp1 += count;
	    }
	}
      *dp2++ = 0xff;
    }
}

int
ShmBitmapSize (int w, int h, void *_dp1)
{
/* Compiled format: <bytes_to_skip (0-254)><number_of_pixels (0-255)> */
/*                  <pixel_data>[<end_of_line(255)>]... */
  uchar          *dp1 = _dp1;
  int             size = 0;
  int             i;
  for (i = 0; i < h; i++)
    {
      int             x = 0;
      while (x < w)
	{
	  int             count;
	  /* count zeroes */
	  count = 0;
	  while (x < w && *(dp1 + count) == notusedc && count < 254)
	    {
	      count++;
	      x++;
	    }
	  size++;
	  dp1 += count;
	  /* count nonzeroes */
	  if (x < w)
	    {
	      count = 0;
	      while (x < w && *(dp1 + count) != notusedc && count < 255)
		{
		  count++;
		  x++;
		}
	      size += count + 1;
	      dp1 += count;
	    }
	}
      size++;
    }
  return size;
}
static void
putmaskcompiledclip (CONST int nx, CONST int ny, CONST int nw, CONST int nh, CONST int _x,
		     CONST int _y, CONST int w, CONST int h, void *_dp)
{
/* Special case costly clipping */
  uchar          *dp = _dp;
  uchar          *vp, *vpline;
  int             y;
  vpline = (uchar *) VBUF + _y * BYTEWIDTH + _x;
  for (y = _y; y < ny + nh; y++)
    {
      int             x = _x;
      vp = vpline;
      for (;;)
	{
	  int             count = *dp++;
	  if (count == 0xff)
	    break;		/* end of line */
	  vp += count;
	  x += count;
	  count = *dp++;
	  /* __memcpy gives severe bug here */
	  if (y >= ny)
	    if (x >= nx)
	      if (x + count > __clipx2 + 1)
		{
		  if (x <= __clipx2)
		    __memcpyb (vp, dp, __clipx2 - x + 1);
		}
	      else
		__memcpyb (vp, dp, count);
	    else if (x + count > __clipx1)
	      if (x + count > __clipx2 + 1)
		__memcpyb (vp + __clipx1 - x,
			   dp + __clipx1 - x,
			   __clipx2 - __clipx1 + 1);
	      else
		__memcpy (vp + __clipx1 - x,
			  dp + __clipx1 - x,
			  count - __clipx1 + x);
	  x += count;
	  vp += count;
	  dp += count;
	}
      vpline += BYTEWIDTH;
    }
}

#define ADJUSTBITMAPBOX() \
	nw = w; nh = h; nx = x; ny = y;				\
	if (nx + nw < __clipx1 || nx > __clipx2)		\
		return;						\
	if (ny + nh < __clipy1 || ny > __clipy2)		\
		return;						\
	if (nx < __clipx1) {		/* left adjust */	\
		nw += nx - __clipx1;				\
		nx = __clipx1;					\
	}							\
	if (ny < __clipy1) {		/* top adjust */	\
		nh += ny - __clipy1;				\
		ny = __clipy1;					\
	}							\
	if (nx + nw > __clipx2)		/* right adjust */	\
		nw = __clipx2 - nx + 1;				\
	if (ny + nh > __clipy2)		/* bottom adjust */	\
		nh = __clipy2 - ny + 1;				\


void
ShmPutBitmap (int x, int y, int w, int h, void *_dp)
{
/* no clipping */
  uchar          *dp = _dp;
  uchar          *vp, *vpline;
  int             i;
  if (Clipping)
    {
      int             nx, ny, nw, nh;
      ADJUSTBITMAPBOX ();
      if (nw != w || nh != h)
	{
	  putmaskcompiledclip (nx, ny, nw, nh, x, y, w, h,
			       dp);
	  return;
	}
    }
  vpline = (uchar *) VBUF + y * BYTEWIDTH + x;
  for (i = 0; i < h; i++)
    {
      vp = vpline;
      for (;;)
	{
	  int             count = *dp++;
	  if (count == 0xff)
	    break;		/* end of line */
	  vp += count;
	  count = *dp++;
	  /* __memcpy gives severe bug here */
	  __memcpyb (vp, dp, count);
	  vp += count;
	  dp += count;
	}
      vpline += BYTEWIDTH;
    }
}


/*following routines are ripped from vgagl library */
/* We use the 32-bit to 64-bit multiply and 64-bit to 32-bit divide of the */
/* 386 (which gcc doesn't know well enough) to efficiently perform integer */
/* scaling without having to worry about overflows. */
#define swap(x, y) { int temp = x; x = y; y = temp; }
#define setpixel (*(backscreen->ff.driver_setpixel_func))
#undef __clipx2
#define __clipx2 (MAPWIDTH-1)
#undef __clipx1
#define __clipx1 0
#undef __clipy1
#define __clipy1 0
#undef __clipy2
#define __clipy2 (MAPHEIGHT+19)
#ifdef __i386__
static INLINE int
muldiv64 (int CONST m1, int CONST m2, int CONST d)
{
/* int32 * int32 -> int64 / int32 -> int32 */
  int             result,dummy;
  __asm__ (
	    "imull %%edx\n\t"
	    "idivl %4\n\t"
:	    "=a" (result), "=d" (dummy)	/* out */
:	    "a" (m1), "d" (m2), "g" (d)		/* in */
    );
  return result;
}

#define INC_IF_NEG(y)                                  \
{                                                       \
        __asm__("btl $31,%1\n\t"                        \
                "adcl $0,%0"                            \
                : "=r" ((int) result)                   \
                : "rm" ((int) (y)), "0" ((int) result)  \
                );                                      \
}
static INLINE int
gl_regioncode (CONST int x, CONST int y)
{
  int             dx1, dx2, dy1, dy2;
  int             result;
  result = 0;
  dy2 = __clipy2 - y;
  INC_IF_NEG (dy2);
  result <<= 1;
  dy1 = y - __clipy1;
  INC_IF_NEG (dy1);
  result <<= 1;
  dx2 = __clipx2 - x;
  INC_IF_NEG (dx2);
  result <<= 1;
  dx1 = x - __clipx1;
  INC_IF_NEG (dx1);
  return result;
}


#else

static INLINE int
gl_regioncode (CONST int x, CONST int y)
{
  int             result = 0;
  if (x < 0)
    result |= 1;
  else if (x > __clipx2)
    result |= 2;
  if (y < 0)
    result |= 4;
  else if (y > __clipy2)
    result |= 8;
  return result;
}
#endif

/* Partly based on vgalib by Tommy Frandsen */
/* This would be a lot faster if setpixel was inlined */

void
Line (int x1, int y1, int x2, int y2, int c)
{
  int             dx, dy, ax, ay, sx, sy, x, y;
  int             syp;
  char           *point;
  if (!shm)
    {
      qLine (x1, y1, x2, y2, c);
      return;
    }
  c = cpixels (c);
  /* Cohen & Sutherland algorithm */
  if (Clipping)
    for (;;)
      {
	int             r1 = gl_regioncode (x1, y1);
	int             r2 = gl_regioncode (x2, y2);
	if (!(r1 | r2))
	  break;		/* completely inside */
	if (r1 & r2)
	  return;		/* completely outside */
	if (r1 == 0)
	  {
	    swap (x1, x2);	/* make sure first */
	    swap (y1, y2);	/* point is outside */
	    r1 = r2;
	  }
	if (r1 & 1)
	  {			/* left */
#ifdef __i386__
	    y1 += muldiv64 (__clipx1 - x1, y2 - y1, x2 - x1);
#else
	    y1 += (long) (__clipx1 - x1) * (long) (y2 - y1) / (long) (x2 - x1);
#endif
	    x1 = __clipx1;
	  }
	else if (r1 & 2)
	  {			/* right */
#ifdef __i386__
	    y1 += muldiv64 (__clipx2 - x1, y2 - y1, x2 - x1);
#else
	    y1 += (long) (__clipx2 - x1) * (long) (y2 - y1) / (long) (x2 - x1);
#endif
	    x1 = __clipx2;
	  }
	else if (r1 & 4)
	  {			/* top */
#ifdef __i386__
	    x1 += muldiv64 (__clipy1 - y1, x2 - x1, y2 - y1);
#else
	    x1 += (long) (__clipy1 - y1) * (long) (x2 - x1) / (long) (y2 - y1);
#endif
	    y1 = __clipy1;
	  }
	else if (r1 & 8)
	  {			/* bottom */
#ifdef __i386__
	    x1 += muldiv64 (__clipy2 - y1, x2 - x1, y2 - y1);
#else
	    x1 += (long) (__clipy2 - y1) * (long) (x2 - x1) / (long) (y2 - y1);
#endif
	    y1 = __clipy2;
	  }
      }
  dx = x2 - x1;
  dy = y2 - y1;
  ax = abs (dx) << 1;
  ay = abs (dy) << 1;
  sx = (dx >= 0) ? 1 : -1;
  sy = (dy >= 0) ? 1 : -1;
  x = x1;
  y = y1;

  point = VScreenToBuffer (backscreen) + x + y * MAPWIDTH;
  if (ax > ay)
    {
      int             d = ay - (ax >> 1);
      syp = sy * MAPWIDTH;
      while (x != x2)
	{
	  *point = c;
	  if (d > 0 || (d == 0 && sx == 1))
	    {
	      y += sy;
	      point += syp;
	      d -= ax;
	    }
	  x += sx;
	  point += sx;
	  d += ay;
	}
    }
  else
    {
      int             sy = (dy >= 0) ? 1 : -1;
      int             d = ax - (ay >> 1);
      syp = sy * MAPWIDTH;
      while (y != y2)
	{
	  *(point) = c;
	  if (d > 0 || (d == 0 && sy == 1))
	    {
	      x += sx;
	      point += sx;
	      d -= ay;
	    }
	  y += sy;
	  point += syp;
	  d += ax;
	}
    }
  *(point) = c;
  point++;
}
#endif
