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
*                   licence details.                       *
*----------------------------------------------------------*
*  init.c jlib depended initialization routines            *
***********************************************************/
#define PLAYFORM_VARIABLES_HERE
#include <vga.h>
#include <vgagl.h>
GraphicsContext *physicalscreen, *backscreen, *starbackground, *background;
void           *fontblack;
void           *fontwhite;

int             VGAMODE = G640x480x256;

int             GAMEWIDTH = 640;
int             GAMEHEIGHT = 460;
int             MAPWIDTH = 640;
int             MAPHEIGHT = 460;
int             DIV = 1;

int flipping=0,page=0;


#include "../koules.h"
#include "../server.h"
#include "../client.h"
#include "../net.h"
#include "../framebuffer.h"
#include <signal.h>
#include <stdlib.h>
#include <alloca.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/vt.h>
#include <unistd.h>
#include <sys/ioctl.h>		/*for waiting for retrace */
extern void     fadein1 ();
extern GraphicsContext *starbackground;
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

static GraphicsContext *
my_allocatecontext ()
{
  return malloc (sizeof (GraphicsContext));
}


char *framebuff;

static void
initialize ()
{
#ifdef SOUND
  printf ("Initializing sound server...\n");
  if (sndinit)
    init_sound ();
#else
  printf ("Sound driver not avaiable-recompile koules with SOUND enabled\n");
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
  printf ("Testing terminal\n");

  printf ("Initializing mouse server\n");
#ifdef MOUSE
  if (!nomouse)
    vga_setmousesupport (1);
#endif
  printf ("Initializing graphics server\n");
  vga_init ();
  if (!vga_hasmode (VGAMODE))
    {
      printf ("graphics mode unavaiable(reconfigure svgalib)\n");
      if (VGAMODE == G640x480x256)
	printf ("or use -s option\n");
      exit (-2);
    }
  vga_setmode (VGAMODE);



  printf ("Initializing video memory\n");
  setcustompalette (0, 1);
  gl_setcontextvga (VGAMODE);


  physicalscreen = my_allocatecontext ();
  backscreen = my_allocatecontext ();
  gl_setclippingwindow (0, 0, MAPWIDTH - 1,
			MAPHEIGHT + 19);
  gl_getcontext (physicalscreen);
  if(flipping) {
    if(physicalscreen->modetype!=CONTEXT_LINEAR||!gl_enablepageflipping(physicalscreen)) flipping=0; else {
    backscreen=physicalscreen;
    flippage();
    }
  }


  printf ("Initializing graphics font\n");
  fontblack = malloc (256 * 8 * 8);
  gl_expandfont (8, 8, back (3), gl_font8x8, fontblack);
  fontwhite = malloc (256 * 8 * 8);
  gl_setfont (8, 8, fontwhite);
  gl_expandfont (8, 8, 255, gl_font8x8, fontwhite);


  gl_write (0, 0, "Graphics daemons fired up");
  gl_write (0, 8, "Checking system consitency....virus not found..");
  if(!flipping) {
  gl_setcontextvgavirtual (VGAMODE);
  gl_setclippingwindow (0, 0, MAPWIDTH - 1,
			MAPHEIGHT + 19);
  }
  gl_getcontext (backscreen);
  gl_setclippingwindow (0, 0, MAPWIDTH - 1,
			MAPHEIGHT + 19);
  gl_setcontext (physicalscreen);
  gl_write (0, 16, "Calibrating delay loop");


  gl_setcontextvgavirtual (VGAMODE);
  background = my_allocatecontext ();
  gl_setclippingwindow (0, 0, MAPWIDTH - 1,
			MAPHEIGHT + 19);
  gl_getcontext (background);
  gl_setcontextvgavirtual (VGAMODE);
  starbackground = my_allocatecontext ();
  gl_setclippingwindow (0, 0, MAPWIDTH - 1,
			MAPHEIGHT + 19);
  gl_getcontext (starbackground);
  gl_setcontext (physicalscreen);
  gl_write (0, 24, "Initializing keyboard daemons");
  if (keyboard_init ())
    {
      printf ("Could not initialize keyboard.\n");
      exit (-1);
    }
  /*keyboard_translatekeys(TRANSLATE_CURSORKEYS | TRANSLATE_KEYPADENTER); */
  keyboard_translatekeys (0);
  gl_write (0, 32, "1 pc capable keyboard found");
}


void flippage(void)
{ int offset=MAPWIDTH * (MAPHEIGHT+20) * page,offset1=MAPWIDTH * (MAPHEIGHT+20)*(page^1);
       if(physicalscreen->modeflags&MODEFLAG_FLIPPAGE_BANKALIGNED) {
	    offset=(offset+0xffff) & ~0xffff;
	    offset1=(offset+0xffff) & ~0xffff;
	}
	vga_setdisplaystart(offset);
	gl_setscreenoffset(offset1);
	page=page^1;
}





void
uninitialize ()
{
  static int      uninitialized = 0;
  if (uninitialized)
    return;
  uninitialized = 1;
/*  int             h, i;
   float           p = 0;
   char            bitmap1[MAPWIDTH][MAPHEIGHT + 20];
   char            bitmap2[MAPWIDTH][MAPHEIGHT + 20];
   if (!(physicalscreen->modeflags & (MODEFLAG_PAGEFLIPPING_ENABLED |
   MODEFLAG_TRIPLEBUFFERING_ENABLED))) {

   gl_enableclipping ();
   gl_setcontext (physicalscreen);
   gl_getbox (0, 0, MAPWIDTH, MAPHEIGHT + 20, bitmap1);
   for (h = (MAPHEIGHT + 20) / 2 - (MAPHEIGHT + 18) / 16; h >= 2; h -= (MAPHEIGHT + 18) / 16)
   {
   p += 64.0 / 8;
   gl_scalebox (MAPWIDTH, MAPHEIGHT + 20, bitmap1,
   MAPWIDTH, h * 2, bitmap2);
   gl_putbox (0, (MAPHEIGHT + 20) / 2 - h, MAPWIDTH, h * 2, bitmap2);
   gl_fillbox (0, (MAPHEIGHT + 20) / 2 - h - (MAPHEIGHT + 20) / 16, MAPWIDTH, (MAPHEIGHT + 20) / 16, 0);
   gl_fillbox (0, (MAPHEIGHT + 20) / 2 + h, MAPWIDTH, (MAPHEIGHT + 20) / 16, 0);
   setcustompalette ((int) p, 1);
   }
   gl_fillbox (0, (MAPHEIGHT + 20) / 2 - 50 - 2 / DIV, MAPWIDTH, 50, 0);
   gl_fillbox (0, (MAPHEIGHT + 20) / 2 + 2 / DIV, MAPWIDTH, 50, 0);
   for (i = MAPWIDTH / 2; i >= 5; i -= 5 / DIV)
   gl_fillbox (0, (MAPHEIGHT + 20) / 2 - 2, MAPWIDTH / 2 - i, 20, 0),
   gl_fillbox (MAPWIDTH / 2 + i, (MAPHEIGHT + 20) / 2 - 2, MAPWIDTH / 2 - i, 20, 0),
   usleep (500);
   } */
  keyboard_close ();
  vga_setmode (TEXT);
#ifdef SOUND
  if (sndinit)
    kill_sound ();
#endif
  printf ("Life support systems disconected\n"
	  "\n\nHave a nice LINUX!\n");
#ifdef NETSUPPORT
  if (client)
    CQuit ("Game uninitialized\n");
#endif
}



static void
uninitializes (int num)
{
  char            s[256];
  sprintf (s, "Signal %i!!!\n", num);
  CQuit (s);
  uninitialize ();
  exit (1);
}



int
main (int argc, char **argv)
{
  char            c;
  nrockets = 1;
  printf ("\n\n\n\n"
	  "                                The  game\n"
	  "                               K O U L E S\n"
	  "                               For svgalib\n"
	  "                               Version:1.4\n"
	  "\n\n\n\n"
	  "                    Copyright(c) Jan Hubicka 1995, 1996\n\n\n");
  while ((c = getopt (argc, argv, "fKWD:P:L:SC:slExMmdh")) != EOF)
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
	    if (sscanf (optarg, "%i", &p) != 1 || p < 0 || p > 4)
	      {
		printf ("-D : invalid difficulty\n");
		exit (0);
	      }
	    difficulty = p;
	  }
	  break;
	case 'P':
	  {
	    int             p;
	    if (sscanf (optarg, "%i", &p) != 1 || p < 0)
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
	    if (sscanf (optarg, "%i", &p) != 1 || p < 1 || p > 100)
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
	  strcpy (servername, optarg);
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
	case 'E':
	  printf ("Network option but no network support compiled\n");
	  break;
#endif

#ifdef NETSUPPORT
	case 'E':
	  server = 1;
	  GAMEWIDTH = 900;
	  GAMEHEIGHT = 600;
	  MAPWIDTH = 900;
	  MAPHEIGHT = 600;
	  DIV = 2;
	  break;
#endif
	case 'f':
	  flipping=1;
	  break;
	case 's':
	  VGAMODE = G320x200x256;
	  GAMEWIDTH = 640;
	  GAMEHEIGHT = 360;
	  MAPWIDTH = 320;
	  MAPHEIGHT = 180;
	  DIV = 2;
	  break;
	case 'l':
	  VGAMODE = G640x480x256;
	  if (GAMEHEIGHT == 360)
	    GAMEHEIGHT = MAPHEIGHT = 360;
	  else
	    MAPHEIGHT = GAMEHEIGHT = 460;
	  GAMEWIDTH = 640;
	  MAPWIDTH = 640;
	  DIV = 1;
	  break;
	case 'x':
	  VGAMODE = G320x240x256;
	  GAMEWIDTH = 640;
	  GAMEHEIGHT = 440;
	  MAPWIDTH = 320;
	  MAPHEIGHT = 220;
	  break;
#ifdef MOUSE
	case 'M':
	  nomouse = 1;
	  drawpointer = 0;
	  break;
#endif
#ifdef SOUND
	case 'd':
	  sndinit = 0;
	  break;
#endif
	default:
	  printf ("USAGE:\n"
		  " -h for help\n"
		  " -f enable experimental page flipping mode\n"
		  " -s for small display(320x200)\n"
		  " -l for large display(640x480)\n"
#ifdef SOUND
		  " -d Disable sound support\n"
#endif
#ifdef MOUSE
		  " -M disable mouse support\n"
#endif
#ifdef NETSUPPORT
		  " -S run koules as network server\n"
		  " -C<host> run koules as network client\n"
		  " -P<port> select port. Default is:%i\n"
		  " -W run server in width mode-support for 320x200 svgalib and OS/2 clients\n"
		  " -L<level> select level for server\n"
		  " -D<number> select dificulty for server:\n"
		  "     0: nightmare\n"
		  "     1: hard\n"
		  "     2: medium(default and recomended)\n"
		  "     3: easy\n"
		  "     4: very easy\n"
		  " -K run server in deathmatch mode\n", DEFAULTINITPORT
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
      atexit (uninitialize);
      signal (SIGHUP, uninitializes);
      signal (SIGINT, uninitializes);
      signal (SIGTRAP, uninitializes);
      signal (SIGABRT, uninitializes);
      signal (SIGSEGV, uninitializes);
      signal (SIGQUIT, uninitializes);
      signal (SIGFPE, uninitializes);
      signal (SIGTERM, uninitializes);
      signal (SIGBUS, uninitializes);
      signal (SIGIOT, uninitializes);
      signal (SIGILL, uninitializes);
      MAPWIDTH = GAMEWIDTH / DIV;
      MAPHEIGHT = GAMEHEIGHT / DIV;

    }
#endif
  printf ("LINUX4GW 1.12.45b professional\n");
  printf ("Copyright(c)1991,1992,1993,1994,1995 Jan Hubicka(JAHUSOFT)\n");
  create_bitmap ();
  initialize ();
  gl_write (0, 40, "Initializing GUI user interface");
#ifdef SOUND
  sound = sndinit;
#endif
  gamemode = MENU;
  gl_write (0, 48, "Initializing 4d rotation tables");
  gl_write (0, 56, "Initializing refresh daemon ");
  gl_write (0, 66, "please wait 12043.21 Bogomipseconds");
  drawbackground ();
  drawstarbackground ();
  gl_setfont (8, 8, fontblack);

  keys[0][0] = SCANCODE_CURSORBLOCKUP;
  keys[0][1] = SCANCODE_CURSORBLOCKDOWN;
  keys[0][2] = SCANCODE_CURSORBLOCKLEFT;
  keys[0][3] = SCANCODE_CURSORBLOCKRIGHT;

  keys[1][0] = SCANCODE_CURSORUP;
  keys[1][1] = SCANCODE_CURSORDOWN;
  keys[1][2] = SCANCODE_CURSORLEFT;
  keys[1][3] = SCANCODE_CURSORRIGHT;
  starwars ();

#ifdef NETSUPPORT
  if (client)
    {
      vga_runinbackground (1);
      client_loop ();
    }
  else
#endif
    game ();
  printf ("uninitializing\n");
  uninitialize ();
  return 0;
}
