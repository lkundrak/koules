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
*  interface.c interface xlib graphics functions           *
***********************************************************/
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>

#include "../koules.h"
#include "../framebuffer.h"
#include <sys/time.h>
#include <unistd.h>
#undef next


struct keyrec  *
findkey (CONST int sc)
{
  struct keyrec  *kkey = ffirstkey;
  while (kkey != NULL && kkey->scancode != sc)
    kkey = kkey->next;
  return (kkey);
}
void
addkey (CONST int scancode)
{
  struct keyrec  *key;
  if (!findkey (scancode))
    {
      key = malloc (sizeof (struct keyrec));
      key->last = NULL;
      key->next = ffirstkey;
      if (ffirstkey != NULL)
	ffirstkey->last = key;
      key->scancode = scancode;
      ffirstkey = key;
    }
}
void
deletekey (CONST int scancode)
{
  struct keyrec  *key;
  if (!(key = findkey (scancode)))
    {
      printf ("Key not found!\n");
      return;
    }
  if (key->last)
    key->last->next = key->next;
  else
    ffirstkey = key->next;
  if (key->next)
    key->next->last = key->last;
  free (key);
}
int
GetKey ()
{
  if (ffirstkey != NULL /* && ffirstkey->next != NULL */ )
    return (ffirstkey->scancode);
  return (0);
}

void
DrawRectangle (CONST int x1, CONST int y1, CONST int x2, CONST int y2, CONST int color)
{
#ifdef MITSHM
  if (shm)
    {
      Line (x1, y1, x2, y1, color);
      Line (x2, y1, x2, y2, color);
      Line (x2, y2, x1, y2, color);
      Line (x1, y2, x1, y1, color);
    }
  else
#endif
    {
      SetColor (color);
      XDrawRectangle (dp, current.pixmap, gc, (int) x1, (int) y1, (int) x2 - x1, (int) y2 - y1);
    }
}

void
ClearScreen ()
{
#ifdef MITSHM
  if (shm && current.pixmap != wi)
    memset (current.vbuff, pixels[0], MAPWIDTH * (MAPHEIGHT + 20));
  else
#endif
    {
      XFillRectangle (dp, current.pixmap, blackgc, 0, 0, MAPWIDTH, MAPHEIGHT + 20);
    }

}
void
CopyVSToVS (CONST VScreenType source, CONST VScreenType destination)
{
#ifdef MITSHM
  if (shm)
#if defined(__i386__)&&defined(ASSEMBLY)
    __memcpy (destination.vbuff, source.vbuff, MAPWIDTH * (MAPHEIGHT + 20));
#else
    memcpy (destination.vbuff, source.vbuff, MAPWIDTH * (MAPHEIGHT + 20));
#endif
  else
#endif
    {
      XCopyArea (dp, source.pixmap, destination.pixmap, whitegc, 0, 0, MAPWIDTH, MAPHEIGHT + 20, 0, 0);
    }
}
void
CopyToScreen (CONST VScreenType source)
{
#ifdef MITSHM
  if (shm)
    {
      switch (depth)
	{
	case 16:
	  {
	    register unsigned char *data = (unsigned char *) source.vbuff,
	                   *end = (unsigned char *) source.vbuff + MAPWIDTH * (MAPHEIGHT + 20);
	    register unsigned short *out = (unsigned short *) source.ximage->data;
	    while (data < end)
	      {
		*out = rpixels[*data];
		*(out + 1) = rpixels[*(data + 1)];
		*(out + 2) = rpixels[*(data + 2)];
		*(out + 3) = rpixels[*(data + 3)];
		*(out + 4) = rpixels[*(data + 4)];
		*(out + 5) = rpixels[*(data + 5)];
		*(out + 6) = rpixels[*(data + 6)];
		*(out + 7) = rpixels[*(data + 7)];
		out += 8;
		data += 8;
	      }
	  }
	  break;
	case 32:
	case 24:
	  {
	    register unsigned char *data = (unsigned char *) source.vbuff,
	                   *end = (unsigned char *) source.vbuff + MAPWIDTH * (MAPHEIGHT + 20);
	    register unsigned long *out = (unsigned long *) source.ximage->data;
	    while (data < end)
	      {
		*out = rpixels[*data];
		*(out + 1) = rpixels[*(data + 1)];
		*(out + 2) = rpixels[*(data + 2)];
		*(out + 3) = rpixels[*(data + 3)];
		*(out + 4) = rpixels[*(data + 4)];
		*(out + 5) = rpixels[*(data + 5)];
		*(out + 6) = rpixels[*(data + 6)];
		*(out + 7) = rpixels[*(data + 7)];
		out += 8;
		data += 8;
	      }
	  }
	  break;
	}
      if (notrealshm)
	XPutImage (dp, wi, whitegc, source.ximage, 0, 0, 0, 0, MAPWIDTH, MAPHEIGHT + 20);
      else
	XShmPutImage (dp, wi, whitegc, source.ximage, 0, 0, 0, 0, MAPWIDTH, MAPHEIGHT + 20, True);
    }
  else
#endif
    {
      if (screenpixmap != source.pixmap)
	XSetWindowBackgroundPixmap (dp, wi, source.pixmap);
      XClearWindow (dp, wi);
    }
}

BitmapType
CompileBitmap (CONST int x, CONST int y, CONST RawBitmapType bitmap)
{
  BitmapType      pm;
#ifdef MITSHM
  int             size1;
  char           *cbitmap;
#endif
#ifdef MITSHM
  if (shm)
    {
      size1 = ShmBitmapSize (x, y, bitmap.vbuff);
      if ((cbitmap = malloc (size1)) == NULL)
	perror ("create_bitmap"), exit (1);
      ShmCompileBitmap (x, y, bitmap.vbuff, cbitmap);
      pm.vbuff = (unsigned char *) cbitmap;
      free (bitmap.vbuff);
    }
  else
#endif
    {
      XFlush (dp);
      pm.bitmap = XCreatePixmap (dp, wi, x, y, DefaultDepth (dp, screen));
      XPutImage (dp, pm.bitmap, whitegc, bitmap.bitmap, 0, 0, 0, 0, x, y);
      pm.mask = XCreateBitmapFromData (dp, wi, bitmap.mask, x, y);
      free (bitmap.mask);
      XSync (dp, 0);
      XDestroyImage (bitmap.bitmap);
    }
  return (pm);
}
RawBitmapType
CreateBitmap (CONST int xv, CONST int yv)
{
  RawBitmapType   img;
  char           *data;
#ifdef MITSHM
  if (shm)
    {
      img.xsize = xv;
      img.vbuff = malloc (xv * yv + 10);
      return (img);
    }
  else
#endif
    {
      if ((data = (char *) calloc ((xv + BitmapPad (dp)) * yv * 4, 1)) == NULL)
	perror ("Memory Error"), exit (2);
      img.bitmap = XCreateImage (dp, DefaultVisual (dp, screen), DefaultDepth (dp, screen),
				 ZPixmap, 0, data, xv, yv,
				 BitmapPad (dp), 0);
      if ((img.mask = (char *) calloc ((xv) * yv, 1)) == NULL)
	perror ("Memory Error"), exit (2);
    }
  return (img);
}
static int      lastcolor = 1;
static int      lastwidth = 1;
static int      needflush = 0;
static int      lastx = 0, lasty = 0;
static Pixmap   lastpixmap = None;
void
FlushParam ()
{
  XGCValues       val;
  val.clip_x_origin = lastx;
  val.clip_y_origin = lasty;
  val.clip_mask = lastpixmap;
  val.foreground = lastcolor;
  val.line_width = lastwidth;
  if (needflush)
    XChangeGC (dp, gc, needflush, &val);
  needflush = 0;

}
void
SetColor (CONST int color)
{
  int             color1 = cpixels (color);
  if (color1 != lastcolor)
    {
      lastcolor = color1;
      if (!needflush)
	{
	  XSetForeground (dp, gc, color1);
	  return;
	}
      else
	{
	  needflush |= GCForeground;
	}
    }
  FlushParam ();
}
void
SetWidth (int width)
{
  if (width == 1)
    width = 0;
  if (lastwidth != width)
    {
      lastwidth = width;
      needflush |= GCLineWidth;
    }
}
void
SetStipple (int x, int y, Pixmap stipple)
{
  if (lastx != x)
    {
      lastx = x;
      needflush |= GCClipXOrigin;
    }
  if (lasty != y)
    {
      lasty = y;
      needflush |= GCClipYOrigin;
    }
  if (lastpixmap != stipple)
    {
      lastpixmap = stipple;
      needflush |= GCClipMask;
    }
  if (needflush)
    FlushParam ();
}
void
UnSetStipple ()
{
  lastpixmap = None;
  needflush |= GCClipMask;
}

void
MyPause ()
{
  XEvent          myEvent;
  tbreak = 1;
  pesc = 0;
  pup = 0;
  pdown = 0;
  pleft = 0;
  pright = 0;
  pp = 0;
  penter = 0;
  while (1)
    {
      XNextEvent (dp, &myEvent);
      switch (myEvent.type)
	{
	case FocusIn:
	  /*if(myEvent.xfocus.detail==NotifyInferior) */
	  return;
	case GraphicsExpose:
	case Expose:
	  CopyToScreen (backscreen);
	}
    }
}
void
ProcessEvents ()
{
  XEvent          myEvent;
  int             rootx, rooty;
  static int      fout = 0;
  Window          rootreturn, childreturn;
  XFlush (dp);
  while (XEventsQueued (dp, QueuedAlready))
    {
      XNextEvent (dp, &myEvent);
      switch (myEvent.type)
	{
	case KeyRelease:
	  if (fout)
	    break;
	  deletekey (XLookupKeysym (&myEvent.xkey, 0));
	  /*printf ("Released:%i\n", myEvent.xkey.keycode); */
	  switch (XLookupKeysym (&myEvent.xkey, 0))
	    {
	    case XK_Escape:
	      pesc = 0;
	      break;
	    case XK_Up:
	      pup = 0;
	      break;
	    case XK_Down:
	      pdown = 0;
	      break;
	    case XK_Left:
	      pleft = 0;
	      break;
	    case XK_Right:
	      pright = 0;
	      break;
	    case XK_H:
	    case XK_h:
	      ph = 0;
	      break;
	    case XK_P:
	    case XK_p:
	    case XK_Pause:
	      pp = 0;
	      break;
	    case XK_KP_Enter:
	    case XK_KP_Space:
	    case XK_Return:
	      penter = 0;
	      break;
	    }
	  break;
	case KeyPress:
	  if (fout)
	    break;
	  addkey (XLookupKeysym (&myEvent.xkey, 0));
	  /*printf ("Pressed:%i\n", myEvent.xkey.keycode); */
	  switch (XLookupKeysym (&myEvent.xkey, 0))
	    {
	    case XK_Escape:
	      pesc = 1;
	      break;
	    case XK_Up:
	      pup = 1;
	      break;
	    case XK_Down:
	      pdown = 1;
	      break;
	    case XK_Right:
	      pright = 1;
	      break;
	    case XK_Left:
	      pleft = 1;
	      break;
	    case XK_H:
	    case XK_h:
	      ph = 1;
	      break;
	    case XK_P:
	    case XK_p:
	    case XK_Pause:
	      pp = 1;
	      break;
	    case XK_KP_Enter:
	    case XK_KP_Space:
	    case XK_Return:
	      penter = 1;
	      break;
	    }
	  break;
	case FocusOut:
	  while (ffirstkey)
	    deletekey (ffirstkey->scancode);
	  /*if(myEvent.xfocus.detail==NotifyPointer) */
	  if (!nopause /*&& gamemode == GAME */  && !client)
	    MyPause ();
	  else
	    fout = 1;
	  break;
	case FocusIn:
	  fout = 0;
	}
    }
  childreturn = 0;
  XQueryPointer (dp, wi, &rootreturn, &childreturn, &rootx, &rooty, &mousex, &mousey, &mousebuttons);
  mousebuttons&=256;
  if (fout)
    mousex = 0,
      mousey = 0,
      mousebuttons = 0;

}



void
PutBitmap (CONST int x, CONST int y, CONST int xsize, CONST int ysize, CONST BitmapType bitmap)
{
#ifdef MITSHM
  if (shm)
    ShmPutBitmap (x, y, xsize, ysize, bitmap.vbuff);
  else
#endif
    {
      XCopyPlane (dp, (bitmap).mask, current.pixmap, maskgc,
		  0, 0, xsize, ysize, x, y, 1);
      XCopyArea (dp, bitmap.bitmap, current.pixmap, orgc,
		 0, 0, xsize, ysize, x, y);

    }
}
