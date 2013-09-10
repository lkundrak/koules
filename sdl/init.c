/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C2013 Lubomir Rintel                                    *
*        Polivkova 10                                      *
*        612 00 Brno                                       *
*        Czech Republic                                    *
*        Phone: 0041-739-669-116                           *
*        eMail: lkundrak@v3.sk                             *
*----------------------------------------------------------*
*   Copyright(c)2013 by Lubomir Rintel.See README for      *
*                     Liecnece details.                    *
*----------------------------------------------------------*
*  sdl/init.c SDL backend support routines                 *
***********************************************************/

#include <interface.h>
#include <stdlib.h>
#include <time.h>
#include <signal.h>

#include <SDL.h>
#include <SDL_gfxPrimitives.h>
#include <SDL_gfxPrimitives_font.h>

#include "../koules.h"
#include "../server.h"
#include "../client.h"
#include "../net.h"
#include "../framebuffer.h"

/* TODO: move these to a common header. */
extern void     starwars ();
extern void     game ();
extern void     setcustompalette (int, float);

/* Game plan. */
#define WIDTH 640
#define HEIGHT 460
#define DIVISOR 1
int             DIV = DIVISOR;
int             MAPWIDTH = WIDTH;
int             MAPHEIGHT = HEIGHT;
int             GAMEWIDTH = WIDTH * DIVISOR;
int             GAMEHEIGHT = HEIGHT * DIVISOR;

SDL_Surface    *sdl_screen = NULL;

static SDL_Surface *
create_icon ()
{
  char            radius = 32 / 2;
  unsigned char   color = 4 * 32;
  RawBitmapType   img;
  int             x, y, r;

  img = CreateBitmap (radius * 2, radius * 2);
  if (img == NULL)
    return NULL;

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
	    BSetPixel (img, x, y, color + r);
	  }
	else
	  {
	    BSetPixel (img, x, y, 0);
	  }
      }
  return img;
}

/* Tear down. */
void
uninitialize (void)
{
  SDL_Quit ();
}

/* Create screen surfaces, a window and set the global font. */
static int
initialize (void)
{
  int             ret;

#ifdef SOUND
  if (sndinit)
    init_sound ();
#endif

  ret = SDL_Init (SDL_INIT_VIDEO | SDL_INIT_NOPARACHUTE);
  if (ret != 0)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      return ret;
    }

  SDL_WM_SetIcon (create_icon (), NULL);
  sdl_screen = SDL_SetVideoMode (MAPWIDTH, MAPHEIGHT + 20, 32, 0);
  if (sdl_screen == NULL)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      return -1;
    }
  SDL_WM_SetCaption ("Koules for SDL version 1.4 by Jan Hubicka", "Koules");

  backscreen = SDL_CreateRGBSurface (SDL_HWSURFACE,
				     sdl_screen->w, sdl_screen->h,
				     sdl_screen->format->BitsPerPixel,
				     sdl_screen->format->Rmask,
				     sdl_screen->format->Gmask,
				     sdl_screen->format->Bmask,
				     sdl_screen->format->Amask);
  if (backscreen == NULL)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      return -1;
    }

  background = SDL_CreateRGBSurface (SDL_HWSURFACE,
				     sdl_screen->w, sdl_screen->h,
				     sdl_screen->format->BitsPerPixel,
				     sdl_screen->format->Rmask,
				     sdl_screen->format->Gmask,
				     sdl_screen->format->Bmask,
				     sdl_screen->format->Amask);
  if (background == NULL)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      return -1;
    }

  starbackground = SDL_CreateRGBSurface (SDL_HWSURFACE,
					 sdl_screen->w, sdl_screen->h,
					 sdl_screen->format->BitsPerPixel,
					 sdl_screen->format->Rmask,
					 sdl_screen->format->Gmask,
					 sdl_screen->format->Bmask,
					 sdl_screen->format->Amask);
  if (starbackground == NULL)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      return -1;
    }

  gfxPrimitivesSetFont (gfxPrimitivesFontdata, 8, 8);
  SDL_ShowCursor (SDL_DISABLE);

  return 0;
}

static void
uninitializes (int num)
{
  char            s[256];
  CQuit (s);
  uninitialize ();
  exit (1);
}

/* Start the ball rolling. */
int
main (int argc, char *argv[])
{
  char            c;

  nrockets = 1;
  drawpointer = 1;

  while ((c = getopt (argc, argv, "KWD:P:L:SC:slExMmdh")) != EOF)
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
	case 's':
	  GAMEWIDTH = 640;
	  GAMEHEIGHT = 360;
	  MAPWIDTH = 320;
	  MAPHEIGHT = 180;
	  DIV = 2;
	  break;
	case 'l':
	  if (GAMEHEIGHT == 360)
	    GAMEHEIGHT = MAPHEIGHT = 360;
	  else
	    MAPHEIGHT = GAMEHEIGHT = 460;
	  GAMEWIDTH = 640;
	  MAPWIDTH = 640;
	  DIV = 1;
	  break;
	case 'x':
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

  setcustompalette (0, 1);

  printf ("Starting MS-DOS...\n\n\n");
  if (initialize () != 0)
    return 1;

  create_bitmap ();

#ifdef SOUND
  sound = sndinit;
#endif
  gamemode = MENU;

  printf ("HIMEM is testing extended memory...");
  fflush (stdout);
  drawbackground ();
  printf ("done.\n");
  printf ("\n");
  drawstarbackground ();
  printf ("A:\\>");
  fflush (stdout);

  keys[0][0] = SDLK_UP;
  keys[0][1] = SDLK_DOWN;
  keys[0][2] = SDLK_LEFT;
  keys[0][3] = SDLK_RIGHT;

  keys[1][0] = SDLK_w;
  keys[1][1] = SDLK_s;
  keys[1][2] = SDLK_a;
  keys[1][3] = SDLK_d;

  starwars ();

#ifdef NETSUPPORT
  if (client)
    client_loop ();
  else
#endif
    game ();

  uninitialize ();
  return 0;
}
