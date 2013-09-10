/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C1995 JAHUSOFT                                          *
*        Jan Hubicka                                       *
*        Dukelskych Bojovniku 1944                         *
*        390 03 Tabor                                      *
*        Czech Republic                                    *
*        Phone: 0041-36-132613                             *
*        eMail: hubicka@limax.paru.cas.cz                  *
*----------------------------------------------------------*
*    Copyright(c)1995,1996 by Jan Hubicka.See README for   *
*                    licence details.                      *
*----------------------------------------------------------*
*  font.c font for the STARWARS SCROLLER                   *
***********************************************************/
/* this file was ripped from: */
/* Zgv v2.5 - GIF, JPEG and PBM/PGM/PPM viewer, for VGA PCs running Linux.
 * Copyright (C) 1993-1995 Russell Marks. See README for license details.
 *
 * font.c - provides a font of sorts for use via svgalib
 * modified for starwars scroler by Jan hubicka (hubicka@limaxp.aru.cas.cz)
 */


/* it's all hard-coded a bit ugly and prasarna!, so you probably won't want
 * much to look at it (hint) :)
 */


#include <stdio.h>
#include <string.h>
#include "font.h"
#include "koules.h"
/*following routines are ripped from vgagl library */
/* We use the 32-bit to 64-bit multiply and 64-bit to 32-bit divide of the */
/* 386 (which gcc doesn't know well enough) to efficiently perform integer */
/* scaling without having to worry about overflows. */
#ifdef FAST_WIDELINE
#define swap(x, y) { int temp = x; x = y; y = temp; }
#define setpixel (*(backscreen->ff.driver_setpixel_func))
#undef __clipx2
#define __clipx2 (MAPWIDTH-3)
#undef __clipx1
#define __clipx1 0
#undef __clipy1
#define __clipy1 0
#undef __clipy2
#define __clipy2 (MAPHEIGHT+19)
#if defined(__i386__)&&defined(ASSEMBLY)
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

static void
gl_wide_line (int x1, int y1, int x2, int y2, int c)
{
  int             dx, dy, ax, ay, sx, sy, x, y;
  int             syp;
  char           *point;
  x1 = (MAPWIDTH / 2 + (x1) * 220 / (1000 - (y1)) / DIV);
  y1 = (int) (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y1)));
  x2 = (int) (MAPWIDTH / 2 + (x2) * 220 / (1000 - (y2)) / DIV);
  y2 = (int) (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y2)));
  /* Cohen & Sutherland algorithm */
  for (;;)
    {
      int             r1 = gl_regioncode (x1, y1);
      int             r2 = gl_regioncode (x2, y2);
      if (!(r1 | r2))
	break;			/* completely inside */
      if (r1 & r2)
	return;			/* completely outside */
      if (r1 == 0)
	{
	  swap (x1, x2);	/* make sure first */
	  swap (y1, y2);	/* point is outside */
	  r1 = r2;
	}
      if (r1 & 1)
	{			/* left */
#if defined(__i386__)&&defined(ASSEMBLY)
	  y1 += muldiv64 (__clipx1 - x1, y2 - y1, x2 - x1);
#else
	  y1 += (long) (__clipx1 - x1) * (long) (y2 - y1) / (long) (x2 - x1);
#endif
	  x1 = __clipx1;
	}
      else if (r1 & 2)
	{			/* right */
#if defined(__i386__)&&defined(ASSEMBLY)
	  y1 += muldiv64 (__clipx2 - x1, y2 - y1, x2 - x1);
#else
	  y1 += (long) (__clipx2 - x1) * (long) (y2 - y1) / (long) (x2 - x1);
#endif
	  x1 = __clipx2;
	}
      else if (r1 & 4)
	{			/* top */
#if defined(__i386__)&&defined(ASSEMBLY)
	  x1 += muldiv64 (__clipy1 - y1, x2 - x1, y2 - y1);
#else
	  x1 += (long) (__clipy1 - y1) * (long) (x2 - x1) / (long) (y2 - y1);
#endif
	  y1 = __clipy1;
	}
      else if (r1 & 8)
	{			/* bottom */
#if defined(__i386__)&&defined(ASSEMBLY)
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
      if (DIV == 1)
	while (x != x2)
	  {
	    *point = c;
	    if (y > 3 * MAPHEIGHT / 5)
	      {
		*(point + 1) = c;
		if (y > 3 * MAPHEIGHT / 4)
		  *(point + 2) = c;
	      }
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
      else
	while (x != x2)
	  {
	    *point = c;
	    if (y > 4 * MAPHEIGHT / 5)
	      *(point + 1) = c;

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
      if (DIV == 1)
	while (y != y2)
	  {
	    *(point) = c;
	    if (y > 3 * MAPHEIGHT / 5)
	      {
		*(point + 1) = c;
		if (y > 3 * MAPHEIGHT / 4)
		  *(point + 2) = c;
	      }
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
      else
	while (y != y2)
	  {
	    *(point) = c;
	    if (y > 4 * MAPHEIGHT / 5)
	      *(point + 1) = c;

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
  if (DIV == 1)
    {
      *(point) = c;
      point++;
      if (y > 3 * MAPHEIGHT / 5)
	{
	  *(point) = c, point++;
	  if (y > 3 * MAPHEIGHT / 4)
	    *(point) = c;
	}
    }
  else
    {
      *(point) = c;
      point++;
      if (y > 4 * MAPHEIGHT / 5)
	*(point) = c;
    }
}
#define gl_putpixel(x,y,c) FillRectangle(x,y,y>3 * MAPHEIGHT / 4 && DIV == 1?2:1,y>3 * MAPHEIGHT / 5 && DIV == 1?2:1,c)



#define TABSIZE  64		/* size of a tab, in pixels */
#define vga_drawline(x1,y1,x2,y2)\
	gl_wide_line(x1,y1,x2,y2,textcolor)
#define vga_drawhline(x1,y1,x2,y2)\
	gl_hline((int)(MAPWIDTH/2+(x1)*220/(1000-(y1))/DIV),(int)(MAPHEIGHT/3+MAPWIDTH*220/(1000-(y1))),\
	        (int)(MAPWIDTH/2+(x2)*220/(1000-(y2))/DIV),\
	        textcolor)
#define vga_drawpixel(x1,y1)\
	gl_putpixel((int)(MAPWIDTH/2+(x1)*220/(1000-(y1))/DIV),(int)(MAPHEIGHT/3+MAPWIDTH*220/(1000-(y1))),textcolor)

#endif /*FAST_WIDELINE */



#ifdef XSUPPORT
#undef Line1
#undef HLine
#undef SSetPixel
XSegment        lines[1000];
int             nlines;
static void
Line1 (int x1, int y1, int x2, int y2, int c)
{
#ifdef MITSHM
  if (shm)
    {
      gl_wide_line (x1, y1, x2, y2, cpixels (c));
      return;
    }
#endif
  x1 = (MAPWIDTH / 2 + (x1) * 220 / (1000 - (y1)) / DIV);
  y1 = (int) (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y1)));
  x2 = (int) (MAPWIDTH / 2 + (x2) * 220 / (1000 - (y2)) / DIV);
  y2 = (int) (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y2)));
  lines[nlines].x1 = x1;
  lines[nlines].y1 = y1;
  lines[nlines].x2 = x2;
  lines[nlines].y2 = y2;
  nlines++;
}
static void
HLine (int x1, int y1, int x2, int c)
{
  x1 = (MAPWIDTH / 2 + (x1) * 220 / (1000 - (y1)) / DIV);
  x2 = (MAPWIDTH / 2 + (x2) * 220 / (1000 - (y1)) / DIV);
  y1 = (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y1)));
#ifdef MITSHM
  if (shm)
    {
      int             i;
      if (x1 > x2)
	i = x1, x1 = x2, x2 = i;
      if (x1 > MAPWIDTH - 1)
	return;
      if (x2 > MAPWIDTH - 1)
	x2 = MAPWIDTH - 1;
      if (x1 < 0)
	x1 = 0;
      if (x2 < 0)
	return;
      if (y1 > MAPHEIGHT + 19)
	return;
      if (y1 < 0)
	return;
      HLine1 (x1, y1, x2, c);
      return;
    }
#endif /*MITSHM */
  lines[nlines].x1 = x1;
  lines[nlines].y1 = y1;
  lines[nlines].x2 = x2;
  lines[nlines].y2 = y1;
  nlines++;
}
static void
SSetPixel (int x1, int y1, int c)
{
  x1 = (MAPWIDTH / 2 + (x1) * 220 / (1000 - (y1)) / DIV);
  y1 = (int) (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y1)));
#ifdef MITSHM
  if (shm)
    {
      if (x1 >= MAPWIDTH - 2)
	return;
      if (x1 < 0)
	return;
      if (y1 >= MAPHEIGHT + 18)
	return;
      if (y1 < 0)
	return;
      SSetPixel1 (x1, y1, c);
      if (y1 > 3 * MAPHEIGHT / 4 && DIV == 1)
	{
	  SSetPixel1 (x1, y1 + 1, c);
	}
      if (y1 > 3 * MAPHEIGHT / 5 && DIV == 1)
	{
	  SSetPixel1 (x1 + 1, y1, c);
	  SSetPixel1 (x1 + 1, y1 + 1, c);
	}

      return;
    }
#endif
  lines[nlines].x1 = x1;
  lines[nlines].y1 = y1;
  if (y1 > 3 * MAPHEIGHT / 5 && DIV == 1)
    x1++;
  if (y1 > 3 * MAPHEIGHT / 4 && DIV == 1)
    x1++;
  lines[nlines].x2 = x1 + 1;
  lines[nlines].y2 = y1 + 1;
  nlines++;
}
#define TABSIZE  64		/* size of a tab, in pixels */
#undef vga_drawline
#undef vga_drawhline
#undef vga_drawpixel
#endif /*XSUPPORT */
#if defined(XSUPPORT)||!defined(FAST_WIDELINE)
#define vga_drawline(x1,y1,x2,y2)\
	Line1((int)x1,y1,x2,y2,textcolor);
#define vga_drawhline(x1,y1,x2,y2)\
	HLine(x1,y1,x2,textcolor)
#define vga_drawpixel(x1,y1)\
	SSetPixel(x1,y1,textcolor)
#endif

static int      go_through_the_motions = 0;
  /* if 1, we don't draw it, just do the width */

static int      stop_after_this_x = NO_CLIP_FONT;

/* never mind. Character building, wasn't it? (groan) */
#if 0
#define BEST extern inline
#else
#define BEST static
#endif
BEST void
fontc_ul (x, y, r)
     CONST int       x, y, r;
{
  int             r34;

  if (go_through_the_motions)
    return;
  r34 = ((r * 3) >> 2);
  vga_drawline (x - r, y, x - r34, y - r34);
  vga_drawline (x - r34, y - r34, x, y - r);
}

BEST void
fontc_ur (x, y, r)
     CONST int       x, y, r;
{
  int             r34;

  if (go_through_the_motions)
    return;
  r34 = ((r * 3) >> 2);
  vga_drawline (x + r, y, x + r34, y - r34);
  vga_drawline (x + r34, y - r34, x, y - r);
}

BEST void
fontc_ll (x, y, r)
     CONST int       x, y, r;
{
  int             r34;

  if (go_through_the_motions)
    return;
  r34 = ((r * 3) >> 2);
  vga_drawline (x - r, y, x - r34, y + r34);
  vga_drawline (x - r34, y + r34, x, y + r);
}

BEST void
fontc_lr (x, y, r)
     CONST int       x, y, r;
{
  int             r34;

  if (go_through_the_motions)
    return;
  r34 = ((r * 3) >> 2);
  vga_drawline (x + r, y, x + r34, y + r34);
  vga_drawline (x + r34, y + r34, x, y + r);
}

static void
fontc_l (x, y, r)
     CONST int       x, y, r;
{
  fontc_ll (x, y, r);
  fontc_lr (x, y, r);
}

static void
fontc_u (x, y, r)
     CONST int       x, y, r;
{
  fontc_ul (x, y, r);
  fontc_ur (x, y, r);
}

static void
fontc_r (x, y, r)
     CONST int       x, y, r;
{
  fontc_ur (x, y, r);
  fontc_lr (x, y, r);
}

static void
fontc_left (x, y, r)
     CONST int       x, y, r;
{
  fontc_ul (x, y, r);
  fontc_ll (x, y, r);
}

static void
fontcircle (x, y, r)
     CONST int       x, y, r;
{
  fontc_u (x, y, r);
  fontc_l (x, y, r);
}

static INLINE char *
myindex (char *s, CONST int c)
{
  for (; *s != 0; s++)
    if (*s == c)
      return (s);
  return (NULL);
}


int
vgadrawtext (x, y, siz, str)
     CONST int       x, y, siz;
     CONST char     *str;
{
  int             f, a, b, c, s1, s2, s3, s4, s5, s6, gap;
  b = y;
  a = x;
  s1 = siz;
  s2 = s1 << 1;
  s3 = s1 + s2;
  s4 = s2 << 1;
  s5 = s4 + s1;
  s6 = s3 << 1;
  gap = s1;
#ifdef SVGALIBSUPPORT
  gl_setclippingwindow (0, 0, MAPWIDTH - 1,
			MAPHEIGHT + 19);

#endif
  if (gap == 0)
    gap = 1;
#ifdef XSUPPORT
  {
    int             pos;
#ifdef MITSHM
    if (!shm)
#endif
      {
	pos = (int) (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y)));
	if (pos > 3 * MAPHEIGHT / 4)
	  SetWidth (3);
	else if (pos > 3 * MAPHEIGHT / 5)
	  SetWidth (2);
	else
	  SetWidth (1);
	SetColor (textcolor);
      }
    nlines = 0;
  }
#endif
  for (f = 0; f < strlen (str); f++)
    {
      /* s3+s4 is the size that an ellipsis will take up (s3), plus the
       * widest possible letter (M = s4).
       */
      if (a - x > stop_after_this_x - s3 - s4)
	{
	  int             tmp;

	  /* print an ellipsis... well, three dots :) */
	  /* blast the width restriction to stop possible infinite recursion */
	  tmp = stop_after_this_x;
	  set_max_text_width (NO_CLIP_FONT);
	  vgadrawtext (a, y, siz, "...");
	  stop_after_this_x = tmp;
	  /* now give up */
	  break;
	}
      c = str[f];

/*** 1st step: cover some common occurances ***/
      if (!go_through_the_motions) {	/* only draw it if we really want to */
	if (myindex ("abdgopq68", c) != NULL)	/* common circle position */
	  fontcircle (a + s1, b + s3, siz);
	else
	  {			/* common part-circle positions */
	    if (myindex ("cehmnrs", c) != NULL)
	      fontc_ul (a + s1, b + s3, siz);
	    if (myindex ("ehmnrBS35", c) != NULL)
	      fontc_ur (a + s1, b + s3, siz);
	    if (myindex ("cetuyCGJOQSUl035", c) != NULL)
	      fontc_ll (a + s1, b + s3, siz);
	    if (myindex ("suyBCDGJOQSU035", c) != NULL)
	      fontc_lr (a + s1, b + s3, siz);
	    /* common line */
	    if (myindex ("BDEFHKLMNPR", c) != NULL)
	      vga_drawline (a, b, a, b + s4);
	  }
      }
/*** 2nd step: fill in rest - this is the *really* long, messy bit :) ***/

/*** 2a: lowercase letters ***/
      if (!go_through_the_motions)
	switch (c)
	  {
	  case 'a':
	    vga_drawline (a + s2, b + s2, a + s2, b + s4);
	    break;
	  case 'b':
	    vga_drawline (a, b, a, b + s4);
	    break;
	  case 'c':
	    vga_drawhline (a + s1, b + s2, a + s2, b + s2);
	    vga_drawhline (a + s1, b + s4, a + s2, b + s4);
	    break;
	  case 'd':
	    vga_drawline (a + s2, b, a + s2, b + s4);
	    break;
	  case 'e':
	    vga_drawline (a, b + s3, a + s2, b + s3);
	    vga_drawhline (a + s1, b + s4, a + s2, b + s4);
	    break;
	  case 'f':
	    fontc_ul (a + s1, b + s1, siz);
	    vga_drawline (a, b + s1, a, b + s4);
	    vga_drawhline (a, b + s2, a + s1, b + s2);
	    break;
	  case 'g':
	    vga_drawline (a + s2, b + s2, a + s2, b + s5);
	    fontc_l (a + s1, b + s5, siz);
	    break;
	  case 'h':
	    vga_drawline (a, b, a, b + s4);
	    vga_drawline (a + s2, b + s3, a + s2, b + s4);
	    break;
	  case 'i':
	    vga_drawpixel (a, b + s1);
	    vga_drawline (a, b + s2, a, b + s4);
	    a += -s1 + 1;
	    break;
	  case 'j':
	    vga_drawline (a + s1, b + s2, a + s1, b + s5);
	    fontc_lr (a, b + s5, siz);
	    vga_drawpixel (a + s1, b + s1);
	    break;
	  case 'k':
	    vga_drawline (a, b, a, b + s4);
	    vga_drawline (a, b + s3, a + s1, b + s2);
	    vga_drawline (a, b + s3, a + s1, b + s4);
	    break;
	  case 'l':
	    vga_drawline (a, b, a, b + s3);
	    break;
	  case 'm':
	    vga_drawline (a, b + s2, a, b + s4);
	    vga_drawline (a + s2, b + s3, a + s2, b + s4);
	    vga_drawline (a + s4, b + s3, a + s4, b + s4);
	    fontc_u (a + s3, b + s3, siz);
	    break;
	  case 'n':
	    vga_drawline (a, b + s2, a, b + s4);
	    vga_drawline (a + s2, b + s3, a + s2, b + s4);
	    break;
	  case 'p':
	    vga_drawline (a, b + s2, a, b + s6);
	    break;
	  case 'q':
	    vga_drawline (a + s2, b + s2, a + s2, b + s6);
	    break;
	  case 'r':
	    vga_drawline (a, b + s2, a, b + s4);
	    break;
	  case 's':
	    vga_drawhline (a, b + s3, a + s2, b + s3);
	    vga_drawhline (a + s1, b + s2, a + s2, b + s2);
	    vga_drawhline (a, b + s4, a + s1, b + s4);
	    break;
	  case 't':
	    vga_drawline (a, b + s1, a, b + s3);
	    vga_drawhline (a, b + s2, a + s1, b + s2);
	    break;
	  case 'u':
	    vga_drawline (a, b + s2, a, b + s3);
	    vga_drawline (a + s2, b + s2, a + s2, b + s4);
	    break;
	  case 'v':
	    vga_drawline (a, b + s2, a + s1, b + s4);
	    vga_drawline (a + s1, b + s4, a + s2, b + s2);
	    break;
	  case 'w':
	    vga_drawline (a, b + s2, a + s1, b + s4);
	    vga_drawline (a + s1, b + s4, a + s2, b + s3);
	    vga_drawline (a + s2, b + s3, a + s3, b + s4);
	    vga_drawline (a + s3, b + s4, a + s4, b + s2);
	    break;
	  case 'x':
	    vga_drawline (a, b + s2, a + s2, b + s4);
	    vga_drawline (a, b + s4, a + s2, b + s2);
	    break;
	  case 'y':
	    vga_drawline (a, b + s2, a, b + s3);
	    vga_drawline (a + s2, b + s2, a + s2, b + s5);
	    fontc_l (a + s1, b + s5, siz);
	    break;
	  case 'z':
	    vga_drawhline (a, b + s2, a + s2, b + s2);
	    vga_drawline (a + s2, b + s2, a, b + s4);
	    vga_drawhline (a, b + s4, a + s2, b + s4);
	    break;

/*** 2b: uppercase letters ***/

	  case 'A':
	    vga_drawline (a, b + s4, a + s1, b);
	    vga_drawline (a + s1, b, a + s2, b + s4);
	    vga_drawline (a + (s1 >> 1), b + s2, a + s2 - (s1 >> 1), b + s2);
	    break;
	  case 'B':
	    fontc_r (a + s1, b + s1, siz);
	    vga_drawhline (a, b, a + s1, b);
	    vga_drawhline (a, b + s2, a + s1, b + s2);
	    vga_drawhline (a, b + s4, a + s1, b + s4);
	    break;
	  case 'C':
	    fontc_u (a + s1, b + s1, siz);
	    vga_drawline (a, b + s1, a, b + s3);
	    break;
	  case 'D':
	    vga_drawhline (a, b, a + s1, b);
	    vga_drawhline (a, b + s4, a + s1, b + s4);
	    fontc_ur (a + s1, b + s1, siz);
	    vga_drawline (a + s2, b + s1, a + s2, b + s3);
	    break;
	  case 'E':
	    vga_drawhline (a, b, a + s2, b);
	    vga_drawhline (a, b + s2, a + s1, b + s2);
	    vga_drawhline (a, b + s4, a + s2, b + s4);
	    break;
	  case 'F':
	    vga_drawhline (a, b, a + s2, b);
	    vga_drawhline (a, b + s2, a + s1, b + s2);
	    break;
	  case 'G':
	    fontc_u (a + s1, b + s1, siz);
	    vga_drawline (a, b + s1, a, b + s3);
	    vga_drawhline (a + s1, b + s2, a + s2, b + s2);
	    vga_drawline (a + s2, b + s2, a + s2, b + s3);
	    break;
	  case 'H':
	    vga_drawhline (a, b + s2, a + s2, b + s2);
	    vga_drawline (a + s2, b, a + s2, b + s4);
	    break;
	  case 'I':
	    vga_drawhline (a, b, a + s2, b);
	    vga_drawline (a + s1, b, a + s1, b + s4);
	    vga_drawhline (a, b + s4, a + s2, b + s4);
	    break;
	  case 'J':
	    vga_drawline (a + s2, b, a + s2, b + s3);
	    break;
	  case 'K':
	    vga_drawline (a + s2, b, a, b + s2);
	    vga_drawline (a, b + s2, a + s2, b + s4);
	    break;
	  case 'L':
	    vga_drawhline (a, b + s4, a + s2, b + s4);
	    break;
	  case 'M':
	    vga_drawline (a, b, a + s1 + (s1 >> 1), b + s2);
	    vga_drawline (a + s1 + (s1 >> 1), b + s2, a + s3, b);
	    vga_drawline (a + s3, b, a + s3, b + s4);
	    a -= s1;
	    break;
	  case 'N':
	    vga_drawline (a, b, a + s2, b + s4);
	    vga_drawline (a + s2, b + s4, a + s2, b);
	    break;
	  case 'Q':
	    vga_drawline (a + s1, b + s3, a + s2, b + s4);
	    /* FALLS THROUGH and adds an O, finishing the Q */
	  case 'O':
	  case '0':		/* all other numbers done later */
	    fontc_u (a + s1, b + s1, siz);
	    vga_drawline (a, b + s1, a, b + s3);
	    vga_drawline (a + s2, b + s1, a + s2, b + s3);
	    break;
	  case 'R':
	    vga_drawline (a + s1, b + s2, a + s2, b + s4);
	    /* FALLS THROUGH and adds a P, finishing the R */
	  case 'P':
	    fontc_r (a + s1, b + s1, siz);
	    vga_drawhline (a, b, a + s1, b);
	    vga_drawhline (a, b + s2, a + s1, b + s2);
	    break;
	  case 'S':
	    fontc_left (a + s1, b + s1, siz);
	    fontc_ur (a + s1, b + s1, siz);
	    break;
	  case 'T':
	    vga_drawhline (a, b, a + s2, b);
	    vga_drawline (a + s1, b, a + s1, b + s4);
	    break;
	  case 'U':
	    vga_drawline (a, b, a, b + s3);
	    vga_drawline (a + s2, b, a + s2, b + s3);
	    break;
	  case 'V':
	    vga_drawline (a, b, a + s1, b + s4);
	    vga_drawline (a + s1, b + s4, a + s2, b);
	    break;
	  case 'W':
	    vga_drawline (a, b, a + s1, b + s4);
	    vga_drawline (a + s1, b + s4, a + s2, b + s2);
	    vga_drawline (a + s2, b + s2, a + s3, b + s4);
	    vga_drawline (a + s3, b + s4, a + s4, b);
	    break;
	  case 'X':
	    vga_drawline (a, b, a + s2, b + s4);
	    vga_drawline (a + s2, b, a, b + s4);
	    break;
	  case 'Y':
	    vga_drawline (a, b, a + s1, b + s2);
	    vga_drawline (a + s2, b, a + s1, b + s2);
	    vga_drawline (a + s1, b + s2, a + s1, b + s4);
	    break;
	  case 'Z':
	    vga_drawhline (a, b, a + s2, b);
	    vga_drawline (a + s2, b, a, b + s4);
	    vga_drawhline (a, b + s4, a + s2, b + s4);
	    break;

/*** 2c: numbers ***/

	    /* 0 already done */
	  case '1':
	    vga_drawline (a, b + s1, a + s1, b);
	    vga_drawline (a + s1, b, a + s1, b + s4);
	    vga_drawhline (a, b + s4, a + s2, b + s4);
	    break;
	  case '2':
	    fontc_u (a + s1, b + s1, siz);
	    vga_drawline (a + s2, b + s1, a, b + s4);
	    vga_drawline (a, b + s4, a + s2, b + s4);
	    break;
	  case '3':
	    fontc_u (a + s1, b + s1, siz);
	    fontc_lr (a + s1, b + s1, siz);
	    break;
	  case '4':
	    vga_drawline (a + s1, b + s4, a + s1, b);
	    vga_drawline (a + s1, b, a, b + s2);
	    vga_drawhline (a, b + s2, a + s2, b + s2);
	    break;
	  case '5':
	    vga_drawline (a + s2, b, a, b);
	    vga_drawline (a, b, a, b + s2);
	    vga_drawline (a, b + s2, a + s1, b + s2);
	    break;
	  case '6':
	    fontc_u (a + s1, b + s1, siz);
	    vga_drawline (a, b + s1, a, b + s3);
	    break;
	  case '7':
	    vga_drawline (a, b, a + s2, b);
	    vga_drawline (a + s2, b, a + s1, b + s4);
	    break;
	  case '9':
	    vga_drawline (a + s2, b, a + s2, b + s4);
	    /* FALLS THROUGH and does top circle of 8 to complete the 9 */
	  case '8':
	    fontcircle (a + s1, b + s1, siz);
	    break;

	    /* 2d: some punctuation (not much!) */

	  case '-':
	    vga_drawhline (a, b + s2, a + s1, b + s2);
	    break;
	  case '+':
	    vga_drawhline (a, b + s2, a + s1, b + s2);
	    vga_drawline (a + s2, b, a + s2, b + s4);
	    break;
	  case '.':
	    vga_drawpixel (a, b + s4);
	    a += -s1 + 1;
	    break;
	  case ',':
	    vga_drawline (a - 1, b + s4 + s1, a, b + s4);
	    break;
	  case '\'':
	    vga_drawline (a, b, a - 1, b + s1);
	    break;
	  case '(':
	    fontc_ul (a + s1, b + s1, siz);
	    fontc_ll (a + s1, b + s3, siz);
	    vga_drawline (a, b + s1, a, b + s3);
	    break;
	  case ')':
	    fontc_ur (a, b + s1, siz);
	    fontc_lr (a, b + s3, siz);
	    vga_drawline (a + s1, b + s1, a + s1, b + s3);
	    break;
	  case '%':
	    vga_drawpixel (a, b);
	    vga_drawpixel (a + s2, b + s4);
	    /* FALLS THROUGH drawing the slash to finish the % */
	  case '/':
	    vga_drawline (a, b + s4, a + s2, b);
	    break;
	  case '?':
	    fontc_u (a + s1, b + s1, siz);
	    vga_drawline (a + s2, b + s1, a + s1, b + s2);
	    vga_drawline (a + s1, b + s2, a + s1, b + s3);
	    vga_drawpixel (a + s1, b + s4);
	    break;
	  }
/*** 3rd part: finally, move along for the next letter ***/
/*** we do this even if go_through_the_motions is set */
      if (myindex ("ltfijk-. (),'", c) != NULL)
	a += s1;
      else
	{
	  if (myindex ("?/%abcdeghnopqrsuvxyzABCDEFGHIJKLNOPQRSTUVXYZ0123456789", c) != NULL)
	    a += s2;
	  else
	    {
	      if (myindex ("mwMW", c) != NULL)
		a += s4;
	      else
		{
		  if (c == 9)
		    {
		      /* congratulations madam, it's a tab */
		      a = ((a / TABSIZE) + 1) * TABSIZE;
		    }
		  else
		    {
		      /* oh, don't know this one. do an underscore */
		      /* (we don't if go_through_the_motions is set) */
		      if (!go_through_the_motions)
			vga_drawhline (a, b + s4, a + s2, b + s4);
		      a += s2;
		    }
		}
	    }
	}
      a += gap;			/* and add a gap */
    }

#ifdef XSUPPORT
#ifdef MITSHM
  if (!shm)
#endif
    XDrawSegments (dp, current.pixmap, gc, lines, nlines),
      SetWidth (1);
#endif
  return (a - x);
}




/* here's a new one - this gets how wide text will be */
int
vgatextsize (sizearg, str)
     CONST int       sizearg;
     CONST char     *str;
{
  int             r;

  go_through_the_motions = 1;
  r = vgadrawtext (0, 0, sizearg, str);
  go_through_the_motions = 0;
  return (r - sizearg);
}


void
set_max_text_width (width)
     CONST int       width;
{
  stop_after_this_x = width;
}
