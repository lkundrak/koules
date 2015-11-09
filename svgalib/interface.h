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
*  interface.h   interface for svgalib                     *
***********************************************************/
#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>
#ifdef MOUSE
#include <vgamouse.h>
#endif
#include <input.h>
#define EYE_RADIUS (DIV==1?5:6)
#define MOUSE_RADIUS 4
#define SVGALIBSUPPORT

typedef char   *RawBitmapType;
typedef char   *BitmapType;
typedef GraphicsContext *VScreenType;


#ifndef PLATFORM_VARIABLES_HERE
extern int      GAMEWIDTH;
extern int      GAMEHEIGHT;
extern int      MAPWIDTH;
extern int      MAPHEIGHT;
extern int      DIV;

extern int	flipping;
extern int	page;

extern void    *fontblack;
extern void    *fontwhite;
extern VScreenType physicalscreen;
extern VScreenType backscreen;
extern VScreenType background;
extern VScreenType starbackground;
#endif


#define Truecolor 0
#define FastAccess 1
#define BitmapToBuffer(bitmap) (bitmap)
#define VScreenToBuffer(screen) (screen)->vbuf
#define SetPalette(palette) gl_setpalette (palette)
#define SSetPixel(x,y,color) gl_setpixel(x,y,color)
#if 0
#if DIV==1
#define BSetPixel(bitmap,x,y,color) *(BitmapToBuffer(bitmap)+(x)+(((y)&0xffffff00)>>1)+\
                                (((y)&0xffffff00)<<1))=color
#define SMySetPixel(screen,x,y,color) *(VScreenToBuffer(screen)+(x)+(((y)&0xffffff00)>>1)+\
                                (((y)&0xffffff00)<<1))=color
/*#define SSetPixel(x,y,color) *((currentcontext.vbuf)+(x)+(y)*MAPWIDTH) )=color */
#else
#define BSetPixel(bitmap,x,y,color) *(BitmapToBuffer(bitmap)+(x)+((y)&0xffffff00)+(((y)\
                                &0xffffff00)>>2))=color
/*#define SSetPixel(x,y,color) *((currentcontext.vbuf)+(x)+((y)*MAPWIDTH))=color */
#define SMySetPixel(screen,x,y,color) *(VScreenToBuffer(screen)+(x)+((y)&0xffffff00)+(((y)\
                                &0xffffff00)>>2))=color
/*mysetpixel is fast setpixel for drawing movers y=realy*256 */
#endif
#else
#define BSetPixel(bitmap,x,y,color) *(BitmapToBuffer(bitmap)+(x)+(y)*MAPWIDTH)=color
#define SMySetPixel(screen,x,y,color) *(VScreenToBuffer(screen)+(x)+((y)>>8)*MAPWIDTH)=color
#endif
#define SetScreen(screen) gl_setcontext (screen)

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

static inline void
DrawRectangle (int x1, int y1, int x2, int y2, int color)
{
  gl_line ((int) x1, (int) y1, (int) x1, (int) y2, color);
  gl_line ((int) x1, (int) y2, (int) x2, (int) y2, color);
  gl_line ((int) x2, (int) y2, (int) x2, (int) y1, color);
  gl_line ((int) x2, (int) y1, (int) x1, (int) y1, color);
}

static inline char *
CompileBitmap (int x, int y, char *bitmap)
{
  int             size1;
  char           *cbitmap;
  size1 = gl_compiledboxmasksize (x, y, bitmap);
  if ((cbitmap = malloc (size1)) == NULL)
    perror ("create_bitmap"), exit (1);
  gl_compileboxmask (x, y, bitmap, cbitmap);
  return (cbitmap);
}
#define FillRectangle(x,y,x1,y1,color) gl_fillbox((x),(y),(x1),(y1),(color))
#define Line(x1,y1,x2,y2,color) gl_line((x1),(y1),(x2),(y2),(color))
#define HLine(x1,y1,x2,color) gl_hline((x1),(y1),(x2),(color))
#define PutBitmap(x,y,xsize,ysize,bitmap) gl_putboxmaskcompiled((x),(y),(xsize),(ysize),(bitmap))
#define WaitRetrace() vga_waitretrace()
#define EnableClipping() gl_enableclipping()
#define DisableClipping() gl_disableclipping()
#define ClearScreen() gl_clearscreen (0)
extern void flippage(void);
static inline void
CopyVSToVS (VScreenType source, VScreenType destination)
{
  SetScreen (source);
  if(!flipping||(destination!=backscreen&&destination!=physicalscreen))
  gl_copyscreen (destination); else {
    gl_copyboxtocontext(0,0,MAPWIDTH,MAPHEIGHT+20,destination,0,0);
  }
}
static inline void
CopyToScreen (VScreenType source)
{
  SetScreen (source);
  if(!flipping) gl_copyscreen (physicalscreen); else
   flippage();
}
#define CharX 8
#define CharY 8
#define FAST_WIDELINE
