/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C1995 JAHUSOFT                                          *
*        Jan Hubicka                                       *
*        Dukelskych Bojovniku 1944                         *
*        390 03 Tabor                                      *
*        Czech Republic                                    *
*        Telefon: (048-I think) (0361) 32613               *
*        eMail: hubicka@limax.paru.cas.cz                  *
*----------------------------------------------------------*
* Copyright(c)1995,96 by Jan Hubicka &                     *
*              Thomas A. K. Kjaer See README for license   *
*                             details.                     *
*----------------------------------------------------------*
* interface.h interface for OS/2 Warp 3.0 & DIVE           *
***********************************************************/
#define INCL_DOS
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#define  _MEERROR_H_
#include <mmioos2.h>                   /* It is from MMPM toolkit           */
#include <dive.h>
#include <fourcc.h>
#include "input.h"
#include <vga.h>
#include <vgagl.h>

#define WaitRetrace() vga_waitretrace()
#define WAIT 1
#define usleep(s) DosSleep((s)/1000)

#define EYE_RADIUS (DIV==1?5:6)
#define MOUSE_RADIUS 4

typedef PBYTE RawBitmapType;
typedef PBYTE BitmapType;
typedef GraphicsContext *VScreenType;

#ifndef PLATFORM_VARIABLES_HERE
extern int      GAMEWIDTH;
extern int      GAMEHEIGHT;
extern int      MAPWIDTH;
extern int      MAPHEIGHT;
extern int      DIV;

extern void    *fontblack;
extern void    *fontwhite;
extern VScreenType physicalscreen;
extern VScreenType backscreen;
extern VScreenType background;
extern VScreenType starbackground;

#define VScreenToBuffer(screen) (screen)->vbuf

#define SMySetPixel(screen,x,y,color) \
*(VScreenToBuffer(screen)+(int)(x)+\
(((((int)(y)>>8))*MAPWIDTH)))=color

#define SetScreen(screen) gl_setcontext (screen)
#define SSetPixel(x,y,color) gl_setpixel(x,y,color)

#define Line(x1,y1,x2,y2,color) gl_line((x1),(y1),(x2),(y2),(color))

#define PutBitmap(x,y,xsize,ysize,bitmap) \
gl_putboxmaskcompiled((x),(y),(xsize),(ysize),(bitmap))

#define SetPalette(palette) gl_setpalette (palette)

#define EnableClipping() gl_enableclipping()
#define DisableClipping() gl_disableclipping()

static inline void
DrawBlackMaskedText (int x, int y, char *text)
{
  gl_setwritemode (WRITEMODE_MASKED);
  gl_setfont (8, 8, fontblack);
  gl_write (x, y, text);
}
static inline void
DrawWhiteMaskedText (int x, int y, char *text)
{
  gl_setwritemode (WRITEMODE_MASKED);
  gl_setfont (8, 8, fontwhite);
  gl_write (x, y, text);
}
static inline void
DrawText (int x, int y, char *text)
{
  gl_setwritemode (WRITEMODE_OVERWRITE);
  gl_setfont (8, 8, fontwhite);
  gl_write (x, y, text);
}

static inline void  DrawRectangle(int x1, int y1, int x2, int y2, int color)
{
  gl_line ((int) x1, (int) y1, (int) x1, (int) y2, color);
  gl_line ((int) x1, (int) y2, (int) x2, (int) y2, color);
  gl_line ((int) x2, (int) y2, (int) x2, (int) y1, color);
  gl_line ((int) x2, (int) y1, (int) x1, (int) y1, color);
}

static inline BYTE*CompileBitmap(int x, int y, BYTE *bitmap)
{
    int size1;
    BYTE *cbitmap;

    size1 = gl_compiledboxmasksize(x,y,bitmap);
    if ((cbitmap = malloc(size1)) == NULL) 
      perror("create_bitmap"), exit(1);

    gl_compileboxmask(x,y,bitmap,cbitmap);
    return (cbitmap);
}

#define FillRectangle(x,y,x1,y1,color) gl_fillbox((x),(y),(x1),(y1),(color))

#define ClearScreen() gl_clearscreen (0)
static inline void
CopyVSToVS (VScreenType source, VScreenType destination)
{
  SetScreen (source);
  gl_copyscreen (destination);
}
static inline void
CopyToScreen (VScreenType source)
{
/*  O.K. we are cheating. It saves us for 1 screen copy. */
/*  SetScreen (source);
    gl_copyscreen (physicalscreen);
*/
    forceBlitting();
}

#define CharX 8
#define CharY 8
#define FAST_WIDELINE
#endif
