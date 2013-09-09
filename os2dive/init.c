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
 *  init.c OS/2 depended initialization routines            *
 ***********************************************************/
#define INCL_DOS
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#include "koules.h"
#include "pm_main.h"

/* the start-up options dialog */
MRESULT EXPENTRY DlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2);


/* dialog vars */
int             dlg_smode = IDD_320x200;
int             dlg_colors = IDD_COLORS;

#define PLATFORM_VARIABLES_HERE
HAB             hab;
HMQ             hmq;


GraphicsContext *physicalscreen, *backscreen, *starbackground, *background;
void           *fontblack;
void           *fontwhite;

int             VGAMODE = G320x200x256;

int             GAMEWIDTH = 640;
int             GAMEHEIGHT = 360;
int             MAPWIDTH = 320;
int             MAPHEIGHT = 180;
int             DIV = 2;

#include "framebuffer.h"
#include <alloca.h>
#include <sys/time.h>
#include <unistd.h>

extern void     points ();
extern void     points1 ();
char            hole_data[HOLE_RADIUS * 2][HOLE_RADIUS * 2];
char            ehole_data[HOLE_RADIUS * 2][HOLE_RADIUS * 2];
extern char     rocketcolor[5];
extern void     setcustompalette (int, float);
extern void     starwars ();
extern void     game ();


#define NCOLORS 32

#define HOLE_XCENTER (2*HOLE_RADIUS-3*HOLE_RADIUS/4)
#define HOLE_YCENTER (2*HOLE_RADIUS-HOLE_RADIUS/4)
#define HOLE_MAX_RADIUS (HOLE_RADIUS/DIV+0.5*HOLE_RADIUS/DIV)
#define HOLE_SIZE_MAX (radius*radius)

#include "newbutton.c"

static GraphicsContext *
my_allocatecontext ()
{
  return malloc (sizeof (GraphicsContext));
}

void 
uninitialize (void)
{
  vga_setmode (TEXT);
}

static void 
initialize (void)
{
#ifdef SOUND
  if (sndinit)
    init_sound ();
#endif

  if (vga_init ())
    {
      WinDestroyMsgQueue (hmq);
      WinTerminate (hab);
      exit (1);
    };
  if (vga_setmode (VGAMODE))
    {
      WinDestroyMsgQueue (hmq);
      WinTerminate (hab);
      exit (1);
    }

  setcustompalette (0, 1);
  gl_setcontextvga (VGAMODE);

  physicalscreen = my_allocatecontext ();
  gl_getcontext (physicalscreen);

  fontblack = malloc (256 * 8 * 8);
  gl_expandfont (8, 8, back (3), gl_font8x8, fontblack);
  fontwhite = malloc (256 * 8 * 8);
  gl_setfont (8, 8, fontwhite);
  gl_expandfont (8, 8, 255, gl_font8x8, fontwhite);


  gl_write (0, 0, "Graphics daemons fired up");
  forceBlitting ();
  DosSleep (200);

  gl_write (0, 8, "Checking system consitency....virus not found..");
  forceBlitting ();
  DosSleep (200);

  gl_setcontextvgavirtual (VGAMODE);
  backscreen = physicalscreen;	/* my_allocatecontext (); */
/*    gl_getcontext (backscreen); */
  gl_setcontext (physicalscreen);
  gl_write (0, 16, "Calibrating delay loop");

  gl_setcontextvgavirtual (VGAMODE);
  background = my_allocatecontext ();
  gl_getcontext (background);
  gl_setcontextvgavirtual (VGAMODE);
  starbackground = my_allocatecontext ();
  gl_getcontext (starbackground);
  gl_setcontext (physicalscreen);

  forceBlitting ();
  DosSleep (200);

  gl_write (0, 24, "Initializing keyboard daemons");
  forceBlitting ();
  DosSleep (200);

  gl_write (0, 32, "1 pc capable keyboard found");
  forceBlitting ();
  DosSleep (200);
}


int 
main (int argc, char *argv[])
{
  /* Initialize the presentation manager, and create a message queue.
     *
     * Must be done before vga_init() and vga_setmode() is called
     *
     * If you don't need them after initialize, you can destroy them.
   */
  hab = WinInitialize (0);
  hmq = WinCreateMsgQueue (hab, 0);

  RegisterNewBtnClass (hab);

  if (WinDlgBox (HWND_DESKTOP, HWND_DESKTOP, DlgProc,
		 NULLHANDLE, IDD_DLGBOX, NULL))
    {
      WinDestroyMsgQueue (hmq);
      WinTerminate (hab);
      exit (0);
    }

  WinDestroyMsgQueue (hmq);
  WinTerminate (hab);

  /* find screen size */
  switch (dlg_smode)
    {

    case IDD_320x200:
      VGAMODE = G320x200x256;
      GAMEWIDTH = 640;
      GAMEHEIGHT = 360;
      MAPWIDTH = 320;
      MAPHEIGHT = 180;
      DIV = 2;
      break;

    case IDD_320x240:
      VGAMODE = G320x240x256;
      GAMEWIDTH = 640;
      GAMEHEIGHT = 440;
      MAPWIDTH = 320;
      MAPHEIGHT = 220;
      DIV = 2;
      break;

    case IDD_640x480:
      VGAMODE = G640x480x256;
      GAMEWIDTH = 640;
      GAMEHEIGHT = 460;
      MAPWIDTH = 640;
      MAPHEIGHT = 460;
      DIV = 1;
      break;
    }

  nrockets = 1;

  create_bitmap ();
  initialize ();

  vga_titleprint (OS2VERSION);

  vga_pm_sendWMcmd (IDM_SNAP, 0);
  vga_pm_sendWMcmd (IDM_CENTER, 0);

  gl_write (0, 40, "Initializing GUI user interface");
  forceBlitting ();
  DosSleep (200);

  gamemode = MENU;
  gl_write (0, 48, "Initializing 4d rotation tables");
  forceBlitting ();
  DosSleep (200);

  gl_write (0, 56, "Initializing refresh daemon ");
  forceBlitting ();
  DosSleep (200);

  gl_write (0, 66, "please wait 12043.21 Bogomipseconds");
  forceBlitting ();
  DosSleep (200);

  drawbackground ();
  drawstarbackground ();
  gl_setfont (8, 8, fontblack);

  /* default scancodes */
  keys[0][0] = 0x11;		/* up */
  keys[0][1] = 0x1f;		/* down */
  keys[0][2] = 0x1e;		/* left */
  keys[0][3] = 0x20;		/* right */

  keys[1][0] = 0x11;		/* w */
  keys[1][1] = 0x1f;		/* s */
  keys[1][2] = 0x1e;		/* a */
  keys[1][3] = 0x20;		/* d */

  starwars ();

  game ();

  uninitialize ();

  DosExit (1, 0);
}

MRESULT EXPENTRY 
DlgProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  SWP             swp;

  switch (msg)
    {
    case WM_INITDLG:
      (VOID) WinCheckButton (hwnd, dlg_smode, TRUE);
      (VOID) WinCheckButton (hwnd, dlg_colors, TRUE);

      (VOID) WinCheckButton (hwnd, IDD_SOUND, sndinit ? TRUE : FALSE);

      (VOID) WinCheckButton (hwnd, IDD_MOUSE, nomouse ? FALSE : TRUE);

      WinQueryWindowPos (hwnd, (PSWP) & swp);
      WinSetWindowPos (hwnd, HWND_DESKTOP,
      (USHORT) ((WinQuerySysValue (HWND_DESKTOP, SV_CXSCREEN) - swp.cx) / 2),
      (USHORT) ((WinQuerySysValue (HWND_DESKTOP, SV_CYSCREEN) - swp.cy) / 2),
		       0, 0, SWP_MOVE);

      /* Take the input focus. */
      WinFocusChange (HWND_DESKTOP, hwnd, 0L);

      return MRFROMSHORT (1);

    case WM_CONTROL:
      if (SHORT1FROMMP (mp1) >= IDD_320x200 &&
	  SHORT1FROMMP (mp1) <= IDD_640x480)
	{
	  dlg_smode = SHORT1FROMMP (mp1);
	}
      else if (SHORT1FROMMP (mp1) >= IDD_COLORS &&
	       SHORT1FROMMP (mp1) <= IDD_BW)
	{
	  dlg_colors = SHORT1FROMMP (mp1);
	}

      return 0;

    case WM_COMMAND:
      switch (COMMANDMSG (&msg)->cmd)
	{
	case DID_OK:
	  WinDismissDlg (hwnd, FALSE);
	  sndinit = sound = WinQueryButtonCheckstate (hwnd, IDD_SOUND);
	  nomouse = !WinQueryButtonCheckstate (hwnd, IDD_MOUSE);
	  return 0;

	case DID_CANCEL:
	  WinDismissDlg (hwnd, TRUE);
	  return 0;
	}
      break;

    }

  return WinDefDlgProc (hwnd, msg, mp1, mp2);
}
