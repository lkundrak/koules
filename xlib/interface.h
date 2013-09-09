/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C1995 JAHUSOFT                                          *
*        Jan Hubicka                                       *
*        Dukelskych Bojovniku 1944                         *
*        390 03 Tabor                                      *
*        Czech Republic                                    *
*        Phone: 0041-0361-32613                            *
*        eMail: hubicka@limax.paru.cas.cz                  *
*----------------------------------------------------------*
*   Copyright(c)1995,1996 by Jan Hubicka.See README for    *
*                    licence details.                      *
*----------------------------------------------------------*
*  interface.h   interface for Xlib.h                      *
***********************************************************/
#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include "inlstring.h"
#ifdef MITSHM
#include <sys/ipc.h>
#include <sys/shm.h>
#include <X11/extensions/XShm.h>
#endif
#include <input.h>
#define XSUPPORT
#define EYE_RADIUS (DIV==1?5:6)
#define MOUSE_RADIUS 4
#define WRITEMODE_OVERWRITE 0
#define WRITEMODE_MASKED 1
typedef struct
  {
    XImage         *bitmap;
    char           *mask;
#ifdef MITSHM
    int             xsize;
    char           *vbuff;
#endif
  }
RawBitmapType;
typedef struct
  {
    Pixmap          bitmap, mask;
#ifdef MITSHM
    unsigned char  *vbuff;
#endif
  }
BitmapType;
typedef struct
  {
    Pixmap          pixmap;
#ifdef MITSHM
    char           *vbuff;
    XImage	   *ximage;
#endif
  }
VScreenType;


#ifndef PLATFORM_VARIABLES_HERE
#define WHERE extern
#else
#define WHERE
#endif
WHERE void      *fontblack;
WHERE void	*fontwhite;
WHERE int	nofade;
WHERE int	monochrome;
WHERE Colormap  colormap;
WHERE int       useprivate;
WHERE int       fadeenable;
WHERE int       nopause;
WHERE int       shm;
WHERE int       depth;
WHERE int       notusedc;
WHERE int       Clipping;
WHERE int       GAMEWIDTH;
WHERE int       GAMEHEIGHT;
WHERE int       MAPWIDTH;
WHERE int       MAPHEIGHT;
WHERE int       DIV;
WHERE Pixmap    screenpixmap;
WHERE VScreenType physicalscreen;
WHERE VScreenType backscreen;
WHERE VScreenType background;
WHERE VScreenType starbackground;
WHERE VScreenType current;
WHERE Display  *dp;
WHERE Window    wi;
WHERE XFontStruct *fs;
WHERE int       screen;
WHERE int       blackwhite;
WHERE int       pixels[256];
WHERE long       rpixels[256];
WHERE int	notrealshm;
WHERE float	spixels[256];
WHERE float	opixels[256];
WHERE GC        gc;
WHERE GC        maskgc;
WHERE GC        whitegc;
WHERE GC        blackgc;
WHERE GC        orgc;
#undef WHERE


#undef FastAccess
#define CharX 8
#define CharY 8
#define NODIRECT
#ifdef MITSHM
extern void     ShmPutBitmap (int, int, int, int, void *);
extern int      ShmBitmapSize (int, int, void *);
extern void     ShmCompileBitmap (int, int, void *, void *);
extern void     shmline (int, int, int, int, int);
extern void SetFont (int,int,void *);
extern void SetWriteMode(int);
extern void FWrite(int,int,char *);
extern void ExpandFont(int,int,int,void *,void *);
extern void DrawBlackMaskedText(int,int,char *);
extern void DrawWhiteMaskedText(int,int,char *);
extern void DrawText(int,int,char *);

#endif

extern void     SetColor (int);
extern void     SetWidth (int);
extern void     SetStipple (int, int, Pixmap);
extern void     UnSetStipple ();
extern void     FlushParam ();
extern BitmapType CompileBitmap (int, int, RawBitmapType);
extern RawBitmapType CreateBitmap (int, int);
extern void     PutBitmap (int, int, int, int, BitmapType);
extern void     DrawRectangle (int, int, int, int, int);
extern void     ClearScreen ();
extern void     CopyVSToVS (VScreenType, VScreenType);
extern void     CopyToScreen (VScreenType);
extern void     DrawBlackMaskedText (int, int, char *);
extern void     DrawWhiteMaskedText (int x, int y, char *);
extern void     DrawText (int, int, char *);

/*
 * unimplemented and unused functions:
 */
/*SetPalette(palette) #define SGetPixel(x,y) */
/*
 * unimplemented and used functions:
 */
#define WaitRetrace() dummy

/*
 * fast interface macros:
 */
#define cpixels(color) pixels[(unsigned char)(color)]
#define FillRectangle(x,y,x1,y1,color) SetColor(color),XFillRectangle(dp,current.pixmap,gc,(int)(x),(int)(y),(int)(x1),(int)(y1))
#define SetScreen(screen) (current=screen)
#define EnableClipping() Clipping=1
#define DisableClipping() Clipping=0


#ifndef MITSHM
#define DrawBlackMaskedText(xp,yp,text) XDrawString (dp, current.pixmap, blackgc, xp, yp + 7, text, strlen (text))
#define DrawText(xp,yp,text) XDrawString (dp, current.pixmap, whitegc, xp, yp + 7, text, strlen (text))
#define DrawWhiteMaskedText(xp,yp,text) XDrawString (dp, current.pixmap, whitegc, xp, yp + 7, text, strlen (text))
#define BSetPixel(bitmap1,x1,y1,color) (XPutPixel ((XImage *)(bitmap1.bitmap), (int)(x1), (int)(y1), cpixels((color))),\
       (color?(*(bitmap1.mask+((int)(x1))/8+(y1*((bitmap1.bitmap->width+7)/8)))|=1<<(((int)(x1))%8)):0))
#define SSetPixel(x1,y1,color) (SetColor(color),XDrawPoint(dp,current.pixmap,gc,(int)(x1),(int)(y1)))
#define HLine(x1,y1,x2,color) (SetColor(color),XDrawLine(dp,current.pixmap,gc,(int)(x1),(int)(y1),(int)(x2),(int)(y1)))
#define Line(x1,y1,x2,y2,color) (SetColor(color),XDrawLine(dp,current.pixmap,gc,(int)(x1),(int)(y1),(int)(x2),(int)(y2)))

#else

#define VScreenToBuffer(screen) screen.vbuff
#define BitmapToBuffer(bitmap) ((char *)bitmap.vbuff)
#define qSSetPixel(x1,y1,color) (SetColor(color),XDrawPoint(dp,current.pixmap,gc,(int)(x1),(int)(y1)))
#define qBSetPixel(bitmap1,x1,y1,color) (XPutPixel ((XImage *)(bitmap1.bitmap), (int)(x1), (int)(y1), cpixels((color))),\
       (color?(*(bitmap1.mask+((int)(x1))/8+(((int)y1)*((bitmap1.bitmap->width+7)/8)))|=1L<<(((int)(x1))%8)):0))
#define qLine(x1,y1,x2,y2,color) (SetColor(color),XDrawLine(dp,current.pixmap,gc,(int)(x1),(int)(y1),(int)(x2),(int)(y2)))
extern void Line(int,int,int,int,int);
#define SMySetPixel(screen,x,y,color) *(VScreenToBuffer(screen)+(int)(x)+(((int)(y))>>8)*MAPWIDTH)=cpixels(color)
#define SSetPixel(x,y,color) (((int)shm)?(*(VScreenToBuffer(current)+(int)(x)+((int)(y))*MAPWIDTH)=cpixels(color)):(qSSetPixel(x,y,color)))
#define BSetPixel(bitmap1,x,y,color) (((int)shm)?(*(BitmapToBuffer(bitmap1)+(int)(x)+((int)(y))*(bitmap1).xsize)=color?cpixels(color):notusedc):(qBSetPixel(bitmap1,(int)x,(int)y,color)))

/*faster for font */
#define SSetPixel1(x,y,color) (*(VScreenToBuffer(current)+(int)(x)+((int)(y))*MAPWIDTH)=cpixels(color))
#define HLine1(x1,y1,x2,color) memset(VScreenToBuffer(current)+(int)(x1)+((int)(y1))*MAPWIDTH,cpixels(color),(int)(x2)-(int)(x1))
#define HLine(x1,y1,x2,color)  memset(VScreenToBuffer(current)+(int)(x1)+((int)(y1))*MAPWIDTH,cpixels(color),(int)(x2)-(int)(x1))
#define FAST_WIDELINE

#endif

