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
*                     licence details.                     *
*----------------------------------------------------------*
*  init.c xlib depended initialization routines            *
***********************************************************/
#define PLATFORM_VARIABLES_HERE
#include <X11/Xlib.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/cursorfont.h>

#include "cursorm.h"
#include "nocursorm.h"
#include "cursorp.h"
#include "../server.h"
#include "../client.h"
#include "../net.h"
#include "../koules.h"
#include "../framebuffer.h"

#include <sys/time.h>
#include <time.h>
#include <unistd.h>
#include "../mygetopt.h"
#include <sys/signal.h>
extern char    *Font8x8;
extern void     fadein1 ();
extern void     points ();
extern void     points1 ();
extern unsigned char rocketcolor[5];
extern void     setcustompalette (int, float);
extern void     starwars ();
extern void     game ();

static int      Sync;
static int      uninitialized = 0;
void            uninitialize ();



static          Pixmap
create_icon ()
{
  char            radius = 24;
  unsigned char            color = 4 * 32;
  XImage         *img;
  char           *data;
  Pixmap          pixmap;
  int             x, y, r;
  if ((data = (char *) calloc ((radius * 2 + 1 + BitmapPad (dp)) * radius * 2 * 4, 1)) == NULL)
    perror ("Memory Error"), exit (2);
  img = XCreateImage (dp, DefaultVisual (dp, screen), DefaultDepth (dp, screen),
		      ZPixmap, 0, data, radius * 2, radius * 2,
		      BitmapPad (dp), 0);
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
	    XPutPixel (img, x, y, cpixels (color + r));
	  }
	else
	  {
	    XPutPixel (img, x, y, pixels[0]);
	  }
      }
  pixmap = XCreatePixmap (dp, wi, radius * 2, radius * 2, DefaultDepth (dp, screen));
  XSync (dp, 1);
  XPutImage (dp, pixmap, gc, img, 0, 0, 0, 0, radius * 2, radius * 2);
  XSync (dp, 1);
  XDestroyImage (img);
  return (pixmap);
}

#ifdef MITSHM
extern int      XShmQueryExtension (Display * dpy);
extern Bool     XShmQueryVersion (Display* dpy, int * major, int * minor,
                                  Bool * sharedpixmaps);
static int      haderror;
static int      (*origerrorhandler) (Display *, XErrorEvent *);


static int
shmerrorhandler (Display * d, XErrorEvent * e)
{
  haderror++;
  if (e->error_code == BadAccess)
    fprintf (stderr, "X: failed to attach shared memory\n");
  else
    (*origerrorhandler) (d, e);
  return (0);
}

static int
badmatcherrorhandler (Display * d, XErrorEvent * e)
{
  haderror++;
  if (e->error_code != BadMatch)
    (*origerrorhandler) (d, e);
  return (0);
}

#define Width MAPWIDTH
#define Height (MAPHEIGHT+20)
XShmSegmentInfo shminfo;
char           *shared_mem;
int             shared_len;
static void
shmfree ()
{
  if (shm && !notrealshm && !uninitialized)
    {
      uninitialized = 1;
      if (shmdt (shminfo.shmaddr) < 0)
	perror ("X shmdt() error");
      if (shmctl (shminfo.shmid, IPC_RMID, 0) < 0)
	perror ("X shmctl(rmid) error");
#ifdef NETSUPPORT
      if (client)
	CQuit ("Game called exit-probably some graphics interface problem.\n");
#endif
    }
}

void
signalerrorhandler (int num)
{
  if (shmdt (shminfo.shmaddr) < 0)
    perror ("X shmdt() error");
  if (shmctl (shminfo.shmid, IPC_RMID, 0) < 0)
    perror ("X shmctl(rmid) error");
  printf ("fatal signal %i received\n", num);
  XShmDetach (dp, &shminfo);
  fadein ();
  uninitialized = 1;
#ifdef NETSUPPORT
  if (client)
    CQuit ("Fatal signal received");
#endif
  exit (0);
}

int
GetImage (VScreenType * pixmap)	/*Work as in shared memory mose but use
				   normal Image handling functions...good
				   for local server that don't support 
				   MITshm (like my metroX) */
{
  int             memsize;
  unsigned char  *data;
  Pixmap          testpixmap;
  long            VfTime = 0;
  long            VendSleep = 0;
  struct timeval  VlastClk;
  struct timeval  VnewClk;
  int             wait = 1;
  int             i;
  gettimeofday (&VlastClk, NULL);
  gettimeofday (&VnewClk, NULL);
  VendSleep = VlastClk.tv_usec;
  VfTime = 1000000 / 25 * 12;
  pixmap->ximage = XCreateImage (dp, DefaultVisual (dp, screen),
				 depth, ZPixmap, 0,
				 NULL, Width, Height, depth, 0);
  if (!pixmap->ximage)
    {
      fprintf (stderr, "Can't get image\n");
      return 0;
    }
  /*Calculate size of shared memory */
  memsize = ((Width + 1) * (Height + 1) * depth / 8);
  if ((data = malloc (memsize)) == NULL)
    {
      printf ("Can't malloc memory for image\n");
      return 0;
    }
  pixmap->ximage->data = (char *) data;
  printf ("Performing connection benchmark....");
  fflush (stdout);
  testpixmap = XCreatePixmap (dp, wi, MAPWIDTH, MAPHEIGHT + 20, DefaultDepth (dp, screen));
  XSync (dp, 0);
  haderror = False;
  origerrorhandler = XSetErrorHandler (badmatcherrorhandler);
  for (i = 0; i < 10 && wait > 0; i++)
    {
      if (!XPutImage (dp, testpixmap, gc, pixmap->ximage, 0, 0, 0, 0,
                      MAPWIDTH, MAPHEIGHT + 20))
        break;
      XSync (dp, 0);
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
    }
  XSync (dp, False);
  XSetErrorHandler (origerrorhandler);
  XFreePixmap (dp, testpixmap);
  if (haderror)
    {
      printf (" XPutImage failed (remote server?)\n");
      return 0;
    }
  if (i == 10)
    {
      printf ("OK\n");
      notrealshm = 1;
      if (depth == 8)		/*if depth=0 we are using fast shm mode(w/o 
				   recalculating) */
	pixmap->vbuff = (char *) data;
      else
	{			/*In other cases we have to transfer image from
				   8bpp to screen's one before displaying */
	  int             i;
	  for (i = 0; i < 256; i++)
	    rpixels[i] = pixels[i], pixels[i] = i, notusedc = 0;
	  pixmap->vbuff = malloc (Width * Height);
	}
      return 1;
    }
  printf ("Slow...using normal mode\n");
  XDestroyImage (pixmap->ximage);
  return 0;
}
static int
GetShmPixmap (VScreenType * pixmap)
{

  /*Get shared image */
  pixmap->ximage = XShmCreateImage (dp, DefaultVisual (dp, screen),
				    depth, ZPixmap, NULL,
				    &shminfo, Width, Height);
  if (!pixmap->ximage)
    {
      fprintf (stderr, "Can't get shared image\n");
      return 0;
    }
  /*Calculate size of shared memory */
  shared_len = (pixmap->ximage->bytes_per_line + 1)
    * (pixmap->ximage->height + 1);
  /*Avoid one nasty hpux's X server bug..... */
  /*Get shared memory id */
  shminfo.shmid = shmget (IPC_PRIVATE, shared_len, IPC_CREAT | 0777);
  if (shminfo.shmid < 0)
    {
      fprintf (stderr, "Couldn't get X shared memory.\n");
      XDestroyImage (pixmap->ximage);
      return 0;
    }
/* We need to protect our shared memory against orphanage if we
   are killed in this function.  */

  shminfo.shmaddr = (char *) shmat (shminfo.shmid, 0, 0);
  shared_mem = (char *) shminfo.shmaddr;
  if (shminfo.shmaddr == (char *) -1)
    {
      fprintf (stderr, "Couldn't attach to X shared memory.\n");
      XDestroyImage (pixmap->ximage);
      shmctl (shminfo.shmid, IPC_RMID, 0);
      return 0;
    }
  shminfo.readOnly = False;
  pixmap->ximage->data = (char *) shared_mem;

  /* Now try to attach it to the X Server */
  XSync (dp, False);
  haderror = False;
  origerrorhandler = XSetErrorHandler (shmerrorhandler);
  if (!XShmAttach (dp, &shminfo))
    {
      printf ("attach failed");
    }
  XSync (dp, False);		/* wait for error or ok */
  XSetErrorHandler (origerrorhandler);
  if (haderror)
    {
      if (shmdt (shminfo.shmaddr) < 0)
	perror ("X shmdt() error");
      if (shmctl (shminfo.shmid, IPC_RMID, 0) < 0)
	perror ("X shmctl rmid() error");
      return 0;
    }
  memset ((void *) shared_mem, 0, shared_len);

  if (depth == 8)		/*if depth=0 we are using fast shm mode(w/o 
				   recalculating) */
    pixmap->vbuff = shared_mem;
  else
    {				/*In other cases we have to transfer image from
				   8bpp to screen's one before displaying */
      int             i;
      for (i = 0; i < 256; i++)
	rpixels[i] = pixels[i], pixels[i] = i, notusedc = 0;
      pixmap->vbuff = malloc (Width * Height);
    }
  signal (SIGHUP, signalerrorhandler);
  signal (SIGINT, signalerrorhandler);
  signal (SIGTRAP, signalerrorhandler);
  signal (SIGABRT, signalerrorhandler);
  signal (SIGSEGV, signalerrorhandler);
  signal (SIGQUIT, signalerrorhandler);
  signal (SIGFPE, signalerrorhandler);
  signal (SIGTERM, signalerrorhandler);
  signal (SIGBUS, signalerrorhandler);
  signal (SIGIOT, signalerrorhandler);
  signal (SIGILL, signalerrorhandler);
  atexit (shmfree);
  return (1);
}
#undef Width
#undef Height
#endif

static void
initialize (char **argv, int argc)
{
  XWMHints       *wmhints;
  XClassHint     *classhints;
  XSizeHints     *sizehints;
  XTextProperty   windowName;
  XTextProperty   iconName;
  char           *title = "Koules for X version 1.4 by Jan Hubicka";
  char           *title1 = "Koules";

  Font            f;
  XGCValues       values;
#ifndef NAS_SOUND
#ifndef RSOUND
  printf ("Initializing sound server...\n");
#ifdef SOUND
  if (sndinit)
    init_sound ();
  else
    sound = 0;
#else
  printf ("Sound driver not avaiable-recompile koules with SOUND enabled\n");
#endif
#endif
#endif
  printf ("Autoprobing hardware\n");
  printf ("Initializing joystick driver\n");
#ifdef JOYSTICK
  joystickdevice[0] = open ("/dev/js0", O_RDONLY);
  if (joystickdevice[0] < 0)
    {
      perror ("Joystick driver");
      printf ("Joystick 1 not avaiable..\n");
      joystickplayer[0] = -1;
    }
  else
    printf ("Joystick 1 initialized\n");
  joystickdevice[1] = open ("/dev/js1", O_RDONLY);
  if (joystickdevice[1] < 0)
    {
      perror ("Joystick driver");
      printf ("Joystick 2 not avaiable..\n");
      joystickplayer[1] = -1;
    }
  else
    printf ("Joystick 2 initialized\n");

#else
  printf ("Joystick driver not avaiable(recompile koules with JOYSTICK enabled )\n");
#endif
  printf ("Connecting X server\n");
  if ((dp = XOpenDisplay (0)) == 0)
    {
      printf ("could not open display\n");
      exit (0);
    }

  if (Sync)
     XSynchronize (dp, 1);
  XSetCloseDownMode (dp, DestroyAll);
  screen = DefaultScreen (dp);
  {
    XVisualInfo template;
    XVisualInfo * vinfo;
    int nitems_return;
    template.visualid = XVisualIDFromVisual (DefaultVisual (dp, DefaultScreen (dp)));
    vinfo = XGetVisualInfo (dp, VisualIDMask, &template, &nitems_return);
    if (nitems_return == 0)
      {
        printf ("X server does not know about the default visual...");
        exit (0);
      }

    if (vinfo->class == PseudoColor && vinfo->depth == 8)
      {
        fadeenable = 1;
      }
    else
      {
        fadeenable = 0;
        printf ("X: Screen doesn't support PseudoColor!\n");
        if (vinfo->class == TrueColor)
          {
            useprivate = 0;
          }
        else if (vinfo->class == GrayScale)
          {
            useprivate = 0;
            monochrome = 1;
          }
        else
          {
            printf ("Unsupported visual! Using slow and reliable mode\n");
#ifdef MITSHM
            shm = 0;
#endif
          }
      }
    depth = vinfo->depth;
    XFree(vinfo);
  }
#ifdef MITSHM
  /* Make sure all is destroyed if killed off */

  /* Make sure we can do PseudoColor colormap stuff */
  if (!shm)
    {
      printf ("Shm support disabled\n");
    }
  else
    {
      /* Check to see if the extensions are supported */
      int major, minor;
      Bool pixmaps;
      if (XShmQueryVersion (dp, &major, &minor, &pixmaps) != 0
           && (major > 1 || (major == 1 && minor >= 1)))
          printf ("Shm mode seems to be possible\n");
      else
        {
          fprintf (stderr, "X server doesn't support MITSHM extension.\n");
          shm = 0;
        }
    }
#endif
  wi = physicalscreen.pixmap = XCreateSimpleWindow (dp, RootWindow (dp, screen), 50, 50, MAPWIDTH, MAPHEIGHT + 20,
						    0, 0, 0);

  if ((f = XLoadFont (dp, "-schumacher-clean-bold-r-normal--8-80-75-75-c-80-*iso*")) == 0)
    {
      printf ("Font -schumacher-clean-bold-r-normal--8-80-75-75-c-80-*iso* does not exist");
    }


  whitegc = XCreateGC (dp, wi, 0, NULL);
  blackgc = XCreateGC (dp, wi, 0, NULL);
  values.function = GXand;
  maskgc = XCreateGC (dp, wi, GCFunction, &values);
  values.function = GXor;
  orgc = XCreateGC (dp, wi, GCFunction, &values);
  gc = XCreateGC (dp, wi, 0, NULL);
  if (!blackwhite)
    {
      if (!useprivate)
	colormap = DefaultColormap (dp, screen);
      else
	{
	  colormap = XCreateColormap (dp, wi,
				    DefaultVisual (dp, XDefaultScreen (dp)),
				      AllocAll);
	  XSetWindowColormap (dp, wi, colormap);

	}
    }
  setcustompalette (0, 1);
  wmhints = XAllocWMHints ();
  /* Various window manager settings */
  wmhints->initial_state = NormalState;
  wmhints->input = True;
  wmhints->flags |= StateHint | InputHint;
  wmhints->icon_pixmap = create_icon ();
  wmhints->flags = IconPixmapHint;


  /* Set the class for this program */
  classhints = XAllocClassHint ();
  classhints->res_name = title1;
  classhints->res_class = title1;

  /* Setup the max and minimum size that the window will be */
  sizehints = XAllocSizeHints ();
  sizehints->flags = PSize | PMinSize | PMaxSize;
  sizehints->min_width = MAPWIDTH;
  sizehints->min_height = MAPHEIGHT + 20;
  sizehints->max_width = MAPWIDTH;
  sizehints->max_height = MAPHEIGHT + 20;
  /* Create the window/icon name properties */
  if (XStringListToTextProperty (&title, 1, &windowName) == 0)
    {
      fprintf (stderr, "X: Cannot create window name resource!\n");
      exit (3);
    }
  if (XStringListToTextProperty (&title1, 1, &iconName) == 0)
    {
      fprintf (stderr, "X: Cannot create window name resource!\n");
      exit (3);
    }

  /* Now set the window manager properties */
  XSetWMProperties (dp, wi, &windowName, &iconName,
		    argv, argc, sizehints, wmhints, classhints);
  XFree ((void *) wmhints);
  XFree ((void *) classhints);
  XFree ((void *) sizehints);

  XSetFont (dp, gc, f);

  XSelectInput (dp, wi, ExposureMask | KeyPress | KeyRelease | ConfigureRequest | FocusChangeMask);

  XSetFont (dp, whitegc, f);
  XSetFont (dp, blackgc, f);
  if (!useprivate)
    {
      XSetBackground (dp, whitegc, BlackPixel (dp, screen));
      XSetForeground (dp, whitegc, WhitePixel (dp, screen));
      XSetForeground (dp, blackgc, BlackPixel (dp, screen));
    }
  else
    {
      XSetBackground (dp, whitegc, 0);
      XSetForeground (dp, whitegc, pixels[255]);
      XSetForeground (dp, blackgc, 0);
    }
  XSetBackground (dp, maskgc, 0xffffffff);
  XSetForeground (dp, maskgc, 0);
#ifdef MITSHM
  if (!shm)
    {
    noshm:;
#endif
      backscreen.pixmap = XCreatePixmap (dp, wi, MAPWIDTH, MAPHEIGHT + 20, DefaultDepth (dp, screen));
      background.pixmap = XCreatePixmap (dp, wi, MAPWIDTH, MAPHEIGHT + 20, DefaultDepth (dp, screen));
      starbackground.pixmap = XCreatePixmap (dp, wi, MAPWIDTH, MAPHEIGHT + 20, DefaultDepth (dp, screen));
#ifdef MITSHM
    }
  else
    {
      starbackground.vbuff = malloc (MAPWIDTH * (MAPHEIGHT + 20));
      background.vbuff = malloc (MAPWIDTH * (MAPHEIGHT + 20));
      if (!GetShmPixmap (&backscreen))
	{
	  if (!GetImage (&backscreen))
	    {
	      shm = 0;
	      goto noshm;
	    }
	}
      fontblack = malloc (256 * 8 * 8);
      ExpandFont (8, 8, cpixels (32), Font8x8, fontblack);
      fontwhite = malloc (256 * 8 * 8);
      ExpandFont (8, 8, cpixels (255), Font8x8, fontwhite);
    }
#endif

#if defined(NAS_SOUND)||defined(RSOUND)
  printf ("Initializing sound server...\n");
  if (sndinit)
    init_sound ();
  else
    sound = 0;
#endif
  if (DefaultDepth (dp, screen) == 1)
    blackwhite = 1;
}








void
uninitialize ()
{
  fadein ();
  if (!uninitialized)
    {
      XUnmapWindow (dp, wi);
#ifdef MITSHM
      if (shm)
	XDestroyImage (backscreen.ximage);
      else
#endif
	XFreePixmap (dp, backscreen.pixmap);
      XSync (dp, False);
#ifdef MITSHM
      if (shm && !notrealshm)
	XShmDetach (dp, &shminfo);
      XSync (dp, False);	/* need server to detach so can remove id */
      uninitialized = 1;
      if (shm && !notrealshm)
	{
	  if (shmdt (shminfo.shmaddr) < 0)
	    perror ("X shmdt() error");
	  if (shmctl (shminfo.shmid, IPC_RMID, 0) < 0)
	    perror ("X shmctl(rmid) error");
	}
#endif
      XCloseDisplay (dp);
#ifdef SOUND
      if (sndinit)
	{
	  kill_sound ();
	}
#endif
      printf ("have a nice X\n");
    }

}







int
main (int argc, char **argv)
{
  int             c;
  XEvent          event;
  nrockets = 1;
  GAMEWIDTH = 640;
  GAMEHEIGHT = 460;
  MAPWIDTH = 640;
  MAPHEIGHT = 460;
  DIV = 1;
#ifdef MITSHM
  shm = 1;
#endif
  drawpointer = 0;
  printf ("\n\n\n\n"
	  "                                The  game\n"
	  "                               K O U L E S\n"
	  "                                  For X\n"
	  "				  Version 1.4\n"
	  "\n\n\n\n"
	  "                     Copyright(c) Jan Hubicka 1995, 1996\n\n\n");
  useprivate = 0;
  while ((c = mygetopt (argc, argv, "KWD:P:L:C:SxyslEMmpdhfb")) != -1)
    {
      switch (c)
	{
#ifdef NETSUPPORT
	case 'K':
	  server = 1;
	  servergameplan = DEATHMATCH;
	  break;
	case 'W':
	  server = 1;
	  GAMEHEIGHT = 360;
	  break;
	case 'D':
	  {
	    int             p;
	    server = 1;
	    if (sscanf (myoptarg, "%i", &p) != 1 || p < 0 || p > 4)
	      {
		printf ("-D : invalid difficulty \n");
		exit (0);
	      }
	    difficulty = p;
	  }
	  break;
	case 'P':
	  {
	    int             p;
	    if (sscanf (myoptarg, "%i", &p) != 1 || p < 0)
	      {
		printf ("-P : invalid port number\n");
		exit (0);
	      }
	    initport = p;
	  }
	  break;
	case 'L':
	  {
	    int             p;
	    server = 1;
	    if (sscanf (myoptarg, "%i", &p) != 1 || p < 1 || p > 100)
	      {
		printf ("-L : invalid level number\n");
		exit (0);
	      }
	    serverstartlevel = p - 1;
	  }
	  break;
	case 'S':
	  server = 1;
	  break;
	case 'C':
	  strcpy (servername, myoptarg);
	  client = 1;
	  break;
#else
	case 'K':
	case 'W':
	case 'P':
	case 'L':
	case 'D':
	case 'S':
	case 'C':
	  printf ("Network option but no network support compiled\n");
	  break;
#endif
	case 'd':
#ifdef SOUND
	  sndinit = 0;
#endif
	  break;
	case 'f':
	  nofade = 1;
	  break;
	case 'm':
	  monochrome = 1;
	  break;
	case 'b':
	  blackwhite = 1;
	  break;
	case 'p':
	  useprivate = 1;
	  break;
	case 'x':
	  drawpointer = 1;
	  break;
	case 'y':
	  Sync = 1;
	  break;
	case 'E':
	  GAMEWIDTH = 900;
	  GAMEHEIGHT = 600;
	  MAPWIDTH = 900;
	  MAPHEIGHT = 600;
	  DIV = 1;
	  break;
	case 's':
	  GAMEWIDTH = 640;
	  if (GAMEHEIGHT == 360)
	    {
	      MAPHEIGHT = 180;
	    }
	  else
	    {
	      GAMEHEIGHT = 460;
	    }
	  MAPWIDTH = 320;
	  MAPHEIGHT = 230;
	  DIV = 2;
	  break;
	case 'l':
	  GAMEWIDTH = 640;
	  if (GAMEHEIGHT == 360)
	    {
	      MAPHEIGHT = 360;
	    }
	  else
	    {
	      GAMEHEIGHT = 460;
	    }
	  MAPWIDTH = 640;
	  MAPHEIGHT = 460;
	  DIV = 1;
	  break;
	case 'M':
#ifdef MITSHM
	  shm = 0;
#endif
	  break;
	default:
#define USAGE_NETSUPPORT " -S run koules as network server\n \
-C<host> run koules as network client\n \
-P<port> select port. Default is:%i\n \
-W run server in width mode-support for 320x200 svgalib and OS/2 clients\n \
-L<level> select level for server\n \
-D<number> select dificulty for server:\n \
    0: nightmare\n \
    1: hard\n \
    2: medium(default and recomended)\n \
    3: easy\n \
    4: very easy\n \
-K run server in deathmatch mode\n "
#ifdef SOUND
#define USAGE_SOUND " -d Disable sound support\n"
#else
#define USAGE_SOUND ""
#endif
#ifdef MITSHM
#define USAGE_MITSHM " -M DISABLE shared memory support\n"
#else
#define USAGE_MITSHM ""
#endif
	  printf ("USAGE:"
		  " -h for help\n"
		  " -b for blackandwhite displays\n"
		  " -m for monochrome displays\n"
		  " -s for small display(320x250)\n"
		  " -l for large display(640x480)\n"
		  " -p use private colormap\n"
		  " -y Synchronize with X(for debugging)\n"
		  " -f nofade(for debugging)\n"
		  USAGE_SOUND
		  " -x Disable X11 pointer\n"
		  USAGE_MITSHM
#ifdef NETSUPPORT
		  USAGE_NETSUPPORT,
		  DEFAULTINITPORT
#endif
	    );
	  exit (2);
	}
    }
  srand (time (NULL));
#ifdef NETSUPPORT
  if (server)
    {
      init_server ();
      server_loop ();
    }
  if (client)
    {
      init_client ();
      MAPWIDTH = GAMEWIDTH / DIV;
      MAPHEIGHT = GAMEHEIGHT / DIV;
    }
#endif
  printf ("X4GW 1.21.21 professional\n");
  printf ("Copyright(c)1991,1992,1993,1994,1995 Jan Hubicka(JAHUSOFT)\n");
  initialize (argv, argc);
  XFlush (dp);
  create_bitmap ();
  if (!shm)
    {
      XFreePixmap (dp, eye_bitmap[1].mask);
      XFreePixmap (dp, eye_bitmap[2].mask);
      XFreePixmap (dp, eye_bitmap[3].mask);
      XFreePixmap (dp, eye_bitmap[4].mask);
      XFreePixmap (dp, rocket_bitmap[1].mask);
      XFreePixmap (dp, rocket_bitmap[2].mask);
      XFreePixmap (dp, rocket_bitmap[3].mask);
      XFreePixmap (dp, rocket_bitmap[4].mask);
      XFreePixmap (dp, lball_bitmap[0].mask);
      XFreePixmap (dp, lball_bitmap[1].mask);
      XFreePixmap (dp, ehole_bitmap.mask);
      eye_bitmap[1].mask = eye_bitmap[0].mask;
      eye_bitmap[2].mask = eye_bitmap[0].mask;
      eye_bitmap[3].mask = eye_bitmap[0].mask;
      eye_bitmap[4].mask = eye_bitmap[0].mask;
      rocket_bitmap[1].mask = rocket_bitmap[0].mask;
      rocket_bitmap[2].mask = rocket_bitmap[0].mask;
      rocket_bitmap[3].mask = rocket_bitmap[0].mask;
      rocket_bitmap[4].mask = rocket_bitmap[0].mask;
      lball_bitmap[0].mask = ball_bitmap.mask;
      lball_bitmap[1].mask = ball_bitmap.mask;
      ehole_bitmap.mask = hole_bitmap.mask;
    }

  if (DIV && !shm)
    {
      current.pixmap = lball_bitmap[0].bitmap;
      DrawBlackMaskedText ((int) BALL_RADIUS - 4, (int) BALL_RADIUS - 4, "A");
      current.pixmap = lball_bitmap[1].bitmap;
      DrawBlackMaskedText ((int) BALL_RADIUS - 4, (int) BALL_RADIUS - 4, "M");
    }
#ifdef SOUND
  sound = sndinit;
#endif
  gamemode = MENU;
  XFlush (dp);
  printf ("creating backgrounds\n");
  drawbackground ();
  XFlush (dp);
  drawstarbackground ();
  printf ("starting intro\n");

  {
    Cursor          cursor;
    Pixmap          cur, mask;
    XColor          white, black;
    if (!drawpointer)
      {
	cur = XCreatePixmapFromBitmapData (dp, wi,
					   (char *) cursorp_bits, cursorp_width, cursorp_height, (unsigned long) 1L, (unsigned long) 0L, (unsigned int) 1);
	mask = XCreatePixmapFromBitmapData (dp, wi,
					    (char *) cursorm_bits, cursorm_width, cursorm_height, (unsigned long) 1L, (unsigned long) 0L, (unsigned int) 1);
      }
    else
      {
	cur = XCreatePixmapFromBitmapData (dp, wi,
					   (char *) nocursorm_bits, nocursorm_width, nocursorm_height, (unsigned long) 1, (unsigned long) 0, (unsigned int) 1);
	mask = XCreatePixmapFromBitmapData (dp, wi,
					    (char *) nocursorm_bits, nocursorm_width, nocursorm_height, (unsigned long) 1, (unsigned long) 0, (unsigned int) 1);
      }
    black.red = 0x0000;
    black.green = 0x0000;
    black.blue = 0x0000;
    white.red = 0xFFFF;
    white.green = 0xFFFF;
    if (blackwhite)
      white.blue = 0xffff;
    else
      white.blue = 0x0000;
    cursor = XCreatePixmapCursor (dp, cur,
				  mask, &black, &white, 0, 0);
    XFreePixmap (dp, cur);
    XFreePixmap (dp, mask);

    XDefineCursor (dp, wi, cursor);
  }

  keys[0][0] = XK_Up;
  keys[0][1] = XK_Down;
  keys[0][2] = XK_Left;
  keys[0][3] = XK_Right;

  keys[1][0] = XK_w;
  keys[1][1] = XK_s;
  keys[1][2] = XK_a;
  keys[1][3] = XK_d;

  XMapWindow (dp, wi);
  XNextEvent (dp, &event);
  XSync (dp, 0);
  UpdateInput ();
  starwars ();
  printf ("starting game\n");

#ifdef NETSUPPORT
  if (client)
    client_loop ();
  else
#endif
    game ();
  printf ("uninitializing\n");
  uninitialize ();
  return 0;
}
