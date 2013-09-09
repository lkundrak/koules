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
*  koules.c main game routines                             *
***********************************************************/
/* Changes for OS/2 Warp with Dive.                        *
 *  Copyright(c)1996 by Thomas A. K. Kjaer                 *
 ***********************************************************/
/* Changes for joystick "accelerate by deflection"         *
 *  (c) 1997 by Ludvik Tesar (Ludv\'{\i}k Tesa\v{r})       *
 ************************LT*********************************/
#include <unistd.h>
#define VARIABLES_HERE
#include "koules.h"
#include "server.h"
#include "client.h"
#include <sys/time.h>
int             nobjects = 8;
int             nrockets = 0;
int             drawpointer = 1;
int             difficulty = 2;
int             cit = 0;
#ifdef NETSUPPORT
int             client = 0, server = 0;
#endif
Object          object[MAXOBJECT];
Point           point[MAXPOINT];


BitmapType      bball_bitmap, apple_bitmap, inspector_bitmap, mouse_bitmap,
                lunatic_bitmap, lball_bitmap[NLETTERS], circle_bitmap,
                hole_bitmap, ball_bitmap, eye_bitmap[MAXROCKETS], rocket_bitmap[MAXROCKETS],
                ehole_bitmap;
unsigned char   rocketcolor[5] =
{96, 160, 64, 96, 128};
#ifdef SOUND
int             sndinit = 1;
#endif

int             lastlevel = 0, maxlevel = 0;
unsigned char   control[MAXROCKETS];
struct control  controls[MAXROCKETS];
int             dosprings = 0;
int             randsprings = 0;
int             nomouse = 0;
int             textcolor;
int             sound = 1;
int             tbreak;
int             gameplan = COOPERATIVE;
int             npoint = 0;
int             gamemode;
int             keys[5][4];
int             rotation[MAXROCKETS];
int             a_bballs, a_rockets, a_balls, a_holes, a_apples, a_inspectors,
                a_lunatics, a_eholes;
#ifdef MOUSE
int             mouseplayer = -1;
#endif
#ifdef JOYSTICK
int             joystickplayer[2] =
{-1, -1};
int             joystickdevice[2] =
{-1, -1};
int             calibrated[2];
int             center[2][2];
float    joystickmul[2]={1.5,1.5};
float    joystickthresh[2]={0.1,0.1};
#endif


float           ROCKET_SPEED = 1.2;
float           BALL_SPEED = 1.2;
float           BBALL_SPEED = 1.2;
float           SLOWDOWN = 0.8;
float           GUMM = 20;


float           BALLM = 3;
float           LBALLM = 3;
float           BBALLM = 8;
float           APPLEM = 34;
float           INSPECTORM = 2;
float           LUNATICM = 3.14;
float           ROCKETM = 4;


void
addpoint (CONST int x, CONST int y, CONST int xp, CONST int yp, CONST int color, CONST int time)
{
  point[npoint].x = x / DIV;
  point[npoint].y = y / DIV;
  point[npoint].xp = xp / DIV;
  point[npoint].yp = yp / DIV;
  point[npoint].time = time;

  point[npoint].color = color;
  npoint++;
  if (npoint >= MAXPOINT)
    npoint = 0;
}
#ifdef XSUPPORT			/*fast code for slow X :) */
#define CCONST 3
#define PCONST 0
XPoint          mypixels[256 >> CCONST][MAXPOINT];
int             nmypixels[256 >> CCONST];
#ifdef MITSHM
void
shmpoints ()
{
  register unsigned int x, y;
  Point          *p, *lp;
  lp = &point[MAXPOINT];
  for (p = point; p < lp; p++)
    {
      if (p->time > 0)
	{
	  p->time--;
	  x = (p->x += p->xp) >> 8;
	  y = (p->y += p->yp);
	  if (x > 0 && x < MAPWIDTH &&
	      y > 0 && y >> 8 < MAPHEIGHT)
	    SMySetPixel (backscreen, x, y, p->color);
	  else
	    p->time = 0;
	}
    }
}
#endif
void
points ()
{
  register unsigned int x, y, c;
  Point          *p, *lp;
#ifdef MITSHM
  if (shm)
    {
      shmpoints ();
      return;
    }
#endif
  for (x = 0; x < 256 >> CCONST; x++)
    nmypixels[x] = 0;
  lp = &point[MAXPOINT];
  for (p = point; p < lp; p++)
    {
      if (p->time > 0)
	{
	  p->time--;
	  x = (p->x += p->xp) >> 8;
	  y = (p->y += p->yp) >> 8;
	  if (x > 0 && x < MAPWIDTH &&
	      y > 0 && y < MAPHEIGHT)
	    {
	      c = p->color >> CCONST;
	      mypixels[c][nmypixels[c]].x = x;
	      mypixels[c][nmypixels[c]].y = y;
	      nmypixels[c]++;
	    }
	  else
	    p->time = 0;
	}
    }
  for (x = 0; x < 256 >> CCONST; x++)
    {
      if (nmypixels[x])
	{
	  SetColor ((x << CCONST) + PCONST);
	  XDrawPoints (dp, current.pixmap, gc, mypixels[x], nmypixels[x], CoordModeOrigin);
	}
    }
}
#else
void
points ()
{
  register unsigned int x, y;
  Point          *p, *lp;
  lp = &point[MAXPOINT];
  for (p = point; p < lp; p++)
    {
      if (p->time > 0)
	{
	  p->time--;
	  x = (p->x += p->xp) >> 8;
	  y = (p->y += p->yp);
	  if (x > 0 && x < MAPWIDTH &&
	      y > 0 && y >> 8 < MAPHEIGHT)
	    SMySetPixel (backscreen, x, y, p->color);
	  else
	    p->time = 0;
	}
    } 
}
#endif

void
points1 ()
{
  Point          *p, *lp;
  lp = &point[MAXPOINT];
  for (p = point; p < lp; p++)
    {
      if (p->time > 0)
	{
	  p->time--;
	  p->x += p->xp;
	  p->y += p->yp;
	}
    }
}

INLINE int
radius (CONST int type)
{
  switch (type)
    {
    case EHOLE:
    case HOLE:
      return (HOLE_RADIUS);
    case ROCKET:
      return (ROCKET_RADIUS);
    case BALL:
    case LBALL:
      return (BALL_RADIUS);
    case BBALL:
      return (BBALL_RADIUS);
    case APPLE:
      return (APPLE_RADIUS);
    case INSPECTOR:
      return (INSPECTOR_RADIUS);
    case LUNATIC:
      return (LUNATIC_RADIUS);
    }
  return (0);
}
static INLINE int
color (CONST int type, CONST int i, CONST int letter)
{
  switch (type)
    {
    case EHOLE:
      return (128);
    case HOLE:
      return (64);
    case ROCKET:
      return (rocketcolor[i]);
    case BALL:
      return (64);
    case LBALL:
      switch (letter)
	{
	case L_ACCEL:
	  return (128);
	case L_GUMM:
	  return (160);
	case L_THIEF:
	  return (192);
	case L_FINDER:
	  return (3 * 32);
	case L_TTOOL:
	  return (3 * 32);
	}

    case BBALL:
      return (128);
    case APPLE:
      return (64);
    case INSPECTOR:
      return (160);
    case LUNATIC:
      return (3 * 32);
    }
  return (0);
}
INLINE float
M (CONST int type)
{
  switch (type)
    {
    case APPLE:
      return (APPLEM);
    case INSPECTOR:
      return (INSPECTORM);
    case LUNATIC:
      return (LUNATICM);
    case HOLE:
    case EHOLE:
      return (BBALLM);
    case ROCKET:
      return (ROCKETM);
    case BALL:
    case LBALL:
      return (BALLM);
    case BBALL:
      return (BBALLM);
    }
  return (0);
}

int
find_possition (float *x, float *y, CONST float radius)
{
  int             x1, y1, i, y2 = 0;
  float           xp, yp;
rerand:;
  x1 = rand () % (GAMEWIDTH - 60) + 30;
  y1 = rand () % (GAMEHEIGHT - 60) + 30;
  for (i = 0; i < nobjects; i++)
    {
      xp = x1 - object[i].x;
      yp = y1 - object[i].y;
      if (xp * xp + yp * yp < (radius + object[i].radius) *
	  (radius + object[i].radius))
	{
	  y2++;
	  if (y2 > 10000)
	    return (0);
	  goto rerand;

	}
    }
  *x = (float) x1;
  *y = (float) y1;
  return (1);
}

INLINE void
normalize (float *x, float *y, float size)
{
  float           length = sqrt ((*x) * (*x) + (*y) * (*y));
  if (length == 0)
    length = 1;
  *x *= size / length;
  *y *= size / length;
}


static void
move_objects ()
{
  int             i;
  for (i = 0; i < nobjects; i++)
    if (object[i].type == CREATOR)
      {
	object[i].time--;
	if (object[i].time <= 0)
	  {
	    Effect (S_CREATOR2, next);
	    object[i].live = object[i].live1;
	    object[i].type = object[i].ctype;
	    if (object[i].type == ROCKET)
	      object[i].time = 200;
	    object[i].radius = radius (object[i].ctype);
	    object[i].M = M (object[i].ctype);
	  }
      }
    else if (object[i].live)
      {
	object[i].x += object[i].fx * (GAMEWIDTH / 640.0 + 1) / 2;
	object[i].y += object[i].fy * (GAMEWIDTH / 640.0 + 1) / 2;
      }
}





static int helpmode;
static void help(int x,int y,int radius,char *text)
{
   int x1=x+radius+2,y1=y-4*DIV,x2=x1+strlen(text)*8*DIV,y2=y1+8*DIV;
   if(helpmode&&x1>0&&x2<=GAMEWIDTH-DIV&&y1>0&&y2<GAMEHEIGHT-DIV) {
   DrawBlackMaskedText(x1/DIV+1,y1/DIV+1,text);
   DrawWhiteMaskedText(x1/DIV,y1/DIV,text);
   }
}
char            str[2];
static void
draw_objects (CONST int draw)
{
  char            s[80];
  int             i;
  if (draw)
    {
      CopyVSToVS (background, backscreen);
      SetScreen (backscreen);

      /* Now draw the objects in backscreen. */

      points ();
      help(0,9,0,"Help - press 'H' to disable");
#ifdef XSUPPORT
#ifdef MITSHM
      if (!shm)
#else
      if (1)
#endif
	{
	  XSegment        lines[MAXOBJECT];
	  int             nlines = 0;
	  for (i = 0; i < nobjects; i++)
	    if (object[i].live && object[i].lineto != -1 && object[object[i].lineto].live)
	      {
		lines[nlines].x1 = object[i].x / DIV;
		lines[nlines].y1 = object[i].y / DIV;
		lines[nlines].x2 = object[object[i].lineto].x / DIV;
		lines[nlines].y2 = object[object[i].lineto].y / DIV;
		nlines++;
	      }
	  SetColor (255);
	  XDrawSegments (dp, current.pixmap, gc, lines, nlines);

	}
      else
#endif
	for (i = 0; i < nobjects; i++)
	  if (object[i].live && object[i].lineto != -1 && object[object[i].lineto].live) {
	    Line (object[i].x / DIV,
		  object[i].y / DIV,
		  object[object[i].lineto].x / DIV,
		  object[object[i].lineto].y / DIV,
		  255);
		help((object[i].x+object[object[i].lineto].x)/2,
		     (object[i].y+object[object[i].lineto].y)/2,
		     2,"Spit");
	  }
      for (i = 0; i < nobjects; i++)
	if (object[i].live)
	  {

	    switch (object[i].type)
	      {
	      case BALL:
		PutBitmap ((int) (object[i].x - BALL_RADIUS) / DIV, (int) (object[i].y - BALL_RADIUS) / DIV,
		 BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, ball_bitmap);
		help(object[i].x,object[i].y,object[i].radius,"Koules");
		break;
	      case LBALL:
		switch (object[i].letter)
		  {
		  case L_ACCEL:
		    PutBitmap ((int) (object[i].x - BALL_RADIUS) / DIV, (int) (object[i].y - BALL_RADIUS) / DIV,
			       BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, lball_bitmap[0]);
		    help(object[i].x,object[i].y,object[i].radius,"Acceleration");
		    break;
		  case L_GUMM:
		    PutBitmap ((int) (object[i].x - BALL_RADIUS) / DIV, (int) (object[i].y - BALL_RADIUS) / DIV,
			       BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, lball_bitmap[1]);
		    help(object[i].x,object[i].y,object[i].radius,"Weight");
		    break;
		  case L_THIEF:
		    PutBitmap ((int) (object[i].x - BALL_RADIUS) / DIV, (int) (object[i].y - BALL_RADIUS) / DIV,
			       BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, lball_bitmap[2]);
		    help(object[i].x,object[i].y,object[i].radius,"Thief");
		    break;
		  case L_FINDER:
		    PutBitmap ((int) (object[i].x - BALL_RADIUS) / DIV, (int) (object[i].y - BALL_RADIUS) / DIV,
			       BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, lball_bitmap[3]);
		    help(object[i].x,object[i].y,object[i].radius,"Goodie");
		    break;
		  case L_TTOOL:
		    PutBitmap ((int) (object[i].x - BALL_RADIUS) / DIV, (int) (object[i].y - BALL_RADIUS) / DIV,
			       BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, lball_bitmap[4]);
		    help(object[i].x,object[i].y,object[i].radius,"Thief toolkit");
		    break;
		  }
#if !defined(XSUPPORT)||defined(MITSHM)
#ifdef MITSHM
		if (DIV == 1 && shm)
#else
		if (DIV == 1)
#endif
		  {
		    str[0] = object[i].letter;
		    DrawBlackMaskedText ((int) object[i].x / DIV - 4, (int) object[i].y / DIV - 4, str);
		  }
#endif
		break;
	      case HOLE:
		EnableClipping ();
		PutBitmap ((int) (object[i].x - HOLE_RADIUS) / DIV, (int) (object[i].y - HOLE_RADIUS) / DIV,
			   HOLE_RADIUS * 2, HOLE_RADIUS * 2, hole_bitmap);
		DisableClipping ();
		help(object[i].x,object[i].y,object[i].radius,"Black hole");
		break;
	      case EHOLE:
		EnableClipping ();
		PutBitmap ((int) (object[i].x - HOLE_RADIUS) / DIV, (int) (object[i].y - HOLE_RADIUS) / DIV,
			   HOLE_RADIUS * 2, HOLE_RADIUS * 2, ehole_bitmap);
		DisableClipping ();
		help(object[i].x,object[i].y,object[i].radius,"Magnetic hole");
		break;
	      case BBALL:
		PutBitmap ((int) (object[i].x - BBALL_RADIUS) / DIV, (int) (object[i].y - BBALL_RADIUS) / DIV,
			   BBALL_RADIUS * 2 / DIV, BBALL_RADIUS * 2 / DIV, bball_bitmap);
		help(object[i].x,object[i].y,object[i].radius,"BBALL!");
		break;
	      case INSPECTOR:
		PutBitmap ((int) (object[i].x - INSPECTOR_RADIUS) / DIV, (int) (object[i].y - INSPECTOR_RADIUS) / DIV,
			   INSPECTOR_RADIUS * 2 / DIV, INSPECTOR_RADIUS * 2 / DIV, inspector_bitmap);
		help(object[i].x,object[i].y,object[i].radius,"Inspector");
		break;
	      case LUNATIC:
		PutBitmap ((int) (object[i].x - LUNATIC_RADIUS) / DIV, (int) (object[i].y - LUNATIC_RADIUS) / DIV,
			   LUNATIC_RADIUS * 2 / DIV, LUNATIC_RADIUS * 2 / DIV, lunatic_bitmap);
		help(object[i].x,object[i].y,object[i].radius,"Lunatic");
		break;
	      case APPLE:
		PutBitmap ((int) (object[i].x - APPLE_RADIUS) / DIV, (int) (object[i].y - APPLE_RADIUS) / DIV,
			   APPLE_RADIUS * 2 / DIV, APPLE_RADIUS * 2 / DIV, apple_bitmap);
		EnableClipping ();
		Line ((int) (object[i].x + 10) / DIV, (int) (object[i].y - APPLE_RADIUS - 10) / DIV,
		      (int) (object[i].x) / DIV, (int) (object[i].y - APPLE_RADIUS + 10) / DIV, 150);
		Line ((int) (object[i].x + 10) / DIV + 1, (int) (object[i].y - APPLE_RADIUS - 10) / DIV,
		      (int) (object[i].x) / DIV + 1, (int) (object[i].y - APPLE_RADIUS + 10) / DIV, 150);
		if (DIV == 1)
		  Line ((int) (object[i].x + 10) / DIV + 2, (int) (object[i].y - APPLE_RADIUS - 10) / DIV,
			(int) (object[i].x) / DIV + 2, (int) (object[i].y - APPLE_RADIUS + 10) / DIV, 150);
		DisableClipping ();
		PutBitmap ((int) (object[i].x - EYE_RADIUS) / DIV,
			   (int) (object[i].y + APPLE_RADIUS - 15) / DIV,
		 EYE_RADIUS * 2 / DIV, EYE_RADIUS * 2 / DIV, eye_bitmap[0]);
		help(object[i].x,object[i].y,object[i].radius,"APPLEPOLISHER");
		break;
	      case ROCKET:
		{
		  int             x1, y1;
		help(object[i].x,object[i].y,object[i].radius,"Player");
		  PutBitmap ((int) (object[i].x - ROCKET_RADIUS) / DIV, (int) (object[i].y - ROCKET_RADIUS) / DIV,
			     ROCKET_RADIUS * 2 / DIV, ROCKET_RADIUS * 2 / DIV, rocket_bitmap[i]);
		  EnableClipping ();
		  if (!object[i].thief)
		    {
		      x1 = object[i].x + sin (object[i].rotation - RAD (30)) * EYE_RADIUS1 - EYE_RADIUS;
		      y1 = object[i].y + cos (object[i].rotation - RAD (30)) * EYE_RADIUS1 - EYE_RADIUS;
		      PutBitmap ((int) (x1 / DIV), (int) (y1 / DIV),
				 (int) (EYE_RADIUS * 2 / DIV), (EYE_RADIUS * 2 / DIV), eye_bitmap[i]);
		      x1 = object[i].x + sin (object[i].rotation + RAD (30)) * EYE_RADIUS1 - EYE_RADIUS;
		      y1 = object[i].y + cos (object[i].rotation + RAD (30)) * EYE_RADIUS1 - EYE_RADIUS;
		      PutBitmap ((int) (x1 / DIV), (int) (y1 / DIV),
				 (int) (EYE_RADIUS * 2 / DIV), (EYE_RADIUS * 2 / DIV), eye_bitmap[i]);
		    }
		  else
		    {
		      x1 = object[i].x + sin (object[i].rotation - RAD (30)) * EYE_RADIUS1 - BALL_RADIUS;
		      y1 = object[i].y + cos (object[i].rotation - RAD (30)) * EYE_RADIUS1 - BALL_RADIUS;

		      PutBitmap ((int) (x1 / DIV), (int) (y1 / DIV),
				 (int) (BALL_RADIUS * 2 / DIV), (BALL_RADIUS * 2 / DIV), lball_bitmap[2]);
		      x1 = object[i].x + sin (object[i].rotation + RAD (30)) * EYE_RADIUS1 - BALL_RADIUS;
		      y1 = object[i].y + cos (object[i].rotation + RAD (30)) * EYE_RADIUS1 - BALL_RADIUS;
		      PutBitmap ((int) (x1 / DIV), (int) (y1 / DIV),
				 (int) (BALL_RADIUS * 2 / DIV), (BALL_RADIUS * 2 / DIV), lball_bitmap[2]);
		    }
		  DisableClipping ();
		}
		break;
	      }
	  }
    }
  /*if draw */
  else
    points1 ();
  switch (gamemode)
    {
    case MENU:
      draw_menu (draw);
      break;
    case KEYS:
      draw_keys (draw);
      break;
#ifdef JOYSTICK
    case JOY:
      draw_joy (draw);
      break;
#endif
    }

#ifdef MOUSE
  if (draw && (gamemode == MENU || (gamemode == GAME && mouseplayer != -1)) &&
      MouseX () >= 0 && MouseY () >= 0 && MouseX () < MAPWIDTH &&
      MouseY () < MAPHEIGHT && drawpointer)
    {
      EnableClipping ();
      if (!nomouse)
	{
	  PutBitmap (MouseX () - MOUSE_RADIUS, MouseY () - MOUSE_RADIUS,
		     MOUSE_RADIUS * 2, MOUSE_RADIUS * 2, mouse_bitmap);
	  DisableClipping ();
	}
    }
#endif
  if (draw)
    {
      EnableClipping ();
      if (gameplan == COOPERATIVE && gamemode == GAME && DIV == 1)
	{
	  sprintf (s, "level: %3i", lastlevel + 1);
	  DrawWhiteMaskedText ((MAPWIDTH / 2 - 38 * 4) / 2 - strlen (s) * 4, MAPHEIGHT + 2, s);
	}
      sprintf (s, " lives: %6i%6i%6i%6i%6i",
	       nrockets >= 1 ? object[0].live1 : 0,
	       nrockets >= 2 ? object[1].live1 : 0,
	       nrockets >= 3 ? object[2].live1 : 0,
	       nrockets >= 4 ? object[3].live1 : 0,
	       nrockets >= 5 ? object[4].live1 : 0);
      DrawWhiteMaskedText (MAPWIDTH / 2 - strlen (s) * 4, MAPHEIGHT + 2, s);
      sprintf (s, "scores: %6i%6i%6i%6i%6i",
	       object[0].score,
	       object[1].score,
	       object[2].score,
	       object[3].score,
	       object[4].score);
      DrawWhiteMaskedText (MAPWIDTH / 2 - strlen (s) * 4, MAPHEIGHT + 11, s);

      /* Copy backscreen to physical screen. */
      CopyToScreen (backscreen);
      fadein ();
      DisableClipping ();
    }
}




void
explosion (CONST int x, CONST int y, CONST int type, CONST int letter, CONST int n)
{
  float           i;
  int             speed;
  int             color1;
  int             radius1 = radius (type);
#ifdef NETSUPPORT
  if (server)
    {
      Explosion (x, y, type, letter, n);
      return;
    }
#endif
  for (i = 0; i < RAD (360); i += RAD (360.0) * DIV * DIV / radius1 / radius1 / M_PI)
    {
      speed = rand () % 3096 + 10;
      if (DIV == 1)
	color1 = color (type, n, letter) + (rand () % 16);
      else
	color1 = color (type, n, letter) + (rand () % 32);
      addpoint (x * 256, y * 256,
		sin (i) * (speed),
		cos (i) * (speed),
		color1,
		rand () % 100 + 10);
    }
}



static void
rocket_destroyed (CONST int player)
{
  int             i, nalive = 0, igagnant = 0;
  if (gamemode == GAME)
    switch (gameplan)
      {
      case DEATHMATCH:
	if (nrockets == 1)
	  return;
	for (i = 0; i < nrockets; i++)
	  if (object[i].type == ROCKET && object[i].live && i != player)
	    {
	      object[i].score += 100;
	      nalive++;
	      igagnant = i;
	    }
	if (nalive == 1)	/* winner bonus */
	  object[igagnant].score += 50;
      }
}



void
destroy (CONST int i)
{
  int             y;
  if (object[i].x - object[i].radius < 0)
    object[i].x = object[i].radius + 1, object[i].fx *= -1;
  if (object[i].y - object[i].radius < 0)
    object[i].y = object[i].radius + 1, object[i].fy *= -1;
  if (object[i].x + object[i].radius > GAMEWIDTH)
    object[i].x = GAMEWIDTH - object[i].radius - 1, object[i].fx *= -1;
  if (object[i].y + object[i].radius > GAMEHEIGHT)
    object[i].y = GAMEHEIGHT - object[i].radius - 1, object[i].fy *= -1;
  switch (object[i].type)
    {
    case LBALL:
      Effect (S_DESTROY_BALL, next);
      object[i].live = 0, explosion (object[i].x, object[i].y, object[i].type, object[i].letter, i);
      if (object[i].letter == L_THIEF && allow_finder ())
	{
	  object[i].live = 1;
	  object[i].letter = L_FINDER;
	}			/* else
				   if (object[i].letter == L_FINDER )
				   {
				   object[i].live = 1;
				   object[i].letter = L_THIEF;
				   } */
      break;
    case APPLE:
      Effect (S_DESTROY_ROCKET, 0);
      object[i].live = 0, explosion (object[i].x, object[i].y, object[i].type, object[i].letter, i);
      break;
    case BALL:
    case EHOLE:
    case BBALL:
    case INSPECTOR:
    case LUNATIC:
      Effect (S_DESTROY_BALL, next);
      if ((y = create_letter ()) != 0)
	{
	  object[i].type = LBALL;
	  object[i].M = LBALLM;
	  switch (y)
	    {
	    case 1:
	      object[i].letter = L_ACCEL;
	      break;
	    case 2:
	      object[i].letter = L_GUMM;
	      break;
	    case 3:
	      object[i].letter = L_THIEF;
	      break;
	    case 4:
	      object[i].letter = L_FINDER;
	      break;
	    case 5:
	      object[i].letter = L_TTOOL;
	      break;
	      /*case 3:
	         object[i].letter = L_MUGG;
	         break;
	         case 4:
	         object[i].letter = L_SLOW;
	         break;
	         case 5:
	         object[i].letter = L_WIZZ;
	         break;
	         case 6:
	         object[i].letter = L_FUCK;
	         break; */
	    }
	}
      else
	object[i].live = 0, explosion (object[i].x, object[i].y, object[i].type, object[i].letter, i);
      break;
    case ROCKET:
      Effect (S_DESTROY_ROCKET, 0);
      object[i].live1--, object[i].live--, explosion (object[i].x, object[i].y, object[i].type, object[i].letter, i);
      rocket_destroyed (i);
      if (object[i].live)
	{
	  /*object[i].x = rand () % (GAMEWIDTH-60)+30;
	     object[i].y = rand () % (GAMEHEIGHT-60)+30; */
	  object[i].fx = 0;
	  object[i].fy = 0;
	  object[i].rotation = 0;
	  object[i].type = ROCKET;
	  object[i].accel = ROCKET_SPEED;
	  creator_rocket (i);
	}
      break;
    }
}




static void
check_limit ()
{
  int             i;
  for (i = 0; i < nobjects; i++)
    if (object[i].live)
      {
	if (object[i].x - object[i].radius < 0 || object[i].x + object[i].radius >= GAMEWIDTH ||
	    object[i].y - object[i].radius <= 0 || object[i].y + object[i].radius >= GAMEHEIGHT)
	  {
	    destroy (i);
	  }
      }
}



/*
 * count number of creatures
 */
static void
update_values ()
{
  int             i;
  a_holes = 0;
  a_rockets = 0;
  a_balls = 0;
  a_bballs = 0;
  a_apples = 0;
  a_eholes = 0;
  a_inspectors = 0;
  a_lunatics = 0;
  for (i = 0; i < nobjects; i++)
    {
      if (object[i].live)
	{
	  switch (object[i].type)
	    {
	    case HOLE:
	      a_holes++;
	      break;
	    case EHOLE:
	      a_eholes++;
	      break;
	    case ROCKET:
	      a_rockets++;
	      break;
	    case LBALL:
	    case BALL:
	      a_balls++;
	      break;
	    case BBALL:
	      a_bballs++;
	      break;
	    case APPLE:
	      a_apples++;
	      break;
	    case INSPECTOR:
	      a_inspectors++;
	      break;
	    case LUNATIC:
	      a_lunatics++;
	      break;
	    }
	}
      if (object[i].type == CREATOR)
	{
	  switch (object[i].ctype)
	    {
	    case BBALL:
	      a_bballs++;
	      break;
	    case HOLE:
	      a_holes++;
	      break;
	    case EHOLE:
	      a_eholes++;
	      break;
	    case ROCKET:
	      a_rockets++;
	      break;
	    case LBALL:
	    case BALL:
	      a_balls++;
	      break;
	    case APPLE:
	      a_apples++;
	      break;
	    case INSPECTOR:
	      a_inspectors++;
	      break;
	    case LUNATIC:
	      a_lunatics++;
	      break;
	    }
	}
    }

}



/*
 * accelerate rocket
 */
void
/* howmuch is between 0 and 1, everything else is cheating */
accel (CONST int i, CONST double howmuch)
{
  int             y;
#ifdef NETSUPPORT
  if (server)
    acceled[i] = 1;
#endif
#ifdef NETSUPPORT
  if (!client)
#endif
    {
      object[i].time = 0;
       object[i].fx += howmuch * sin (object[i].rotation) * object[i].accel,
       object[i].fy += howmuch * cos (object[i].rotation) * object[i].accel;
#ifdef NETSUPPORT
      if (!server)
#endif
	for (y = 0; y < 5 / DIV / DIV; y++)
	  {
	    float           p;
	    p = RAD (rand () % 45 - 22);
	    addpoint (object[i].x * 256,
		      object[i].y * 256,
		      (object[i].fx - howmuch * sin (object[i].rotation + p) * object[i].accel * 10) * (rand () % 512),
		      (object[i].fy - howmuch * cos (object[i].rotation + p) * object[i].accel * 10) * (rand () % 512),
		      rocket (rand () % 16), 10);
	  }
    }
#ifdef NETSUPPORT
  else
    {
      for (y = 0; y < 5 / DIV / DIV; y++)
	{
	  float           p;
	  p = RAD (rand () % 30 - 15);
	  addpoint (object[i].x * 256,
		    object[i].y * 256,
		    (-sin (object[i].rotation + p) * ROCKET_SPEED * 5) * (rand () % 512),
		    (-cos (object[i].rotation + p) * ROCKET_SPEED * 5) * (rand () % 512),
		    rocket (rand () % 16), 10);
	}
    }
#endif
}


static void
sprocess_keys ()
{
  int             i;
  if (gamemode != GAME)
    return;
  for (i = 0; i < MAXROCKETS; i++)
    {
      if (object[i].live && object[i].type == ROCKET)
	{
	  switch (controls[i].type)
	    {
#ifdef JOYSTICK
	    case C_JOYSTICK1:
	      {
		double          a, x = controls[i].jx, y = controls[i].jy;
		a = atan (fabs (y) / fabs (x));
		if (x < 0 && y >= 0)
		  object[i].rotation = a + RAD (90);
		else if (x < 0 && y < 0)
		  object[i].rotation = RAD (90) - a;
		else if (x >= 0 && y < 0)
		  object[i].rotation = a + RAD (270);
		else if (x >= 0 && y >= 0)
		  object[i].rotation = RAD (270) - a;
		/* Measure the deflection (a is betw. 0 and 1) */
  	        a=hypot(x*object[i].joymulx,y*object[i].joymuly);
           	/* I must make sure, that I am not cheating :-)  */
		/* "a" can't be bigger than one */
	        if((a>1.0)||(controls[i].mask!=0))a=1.0;
	        if(a>object[i].joythresh)accel(i,a);
	      }
	      break;
#endif
	    case C_MOUSE:
	      {
		double          dx, dy, a;
		dx = object[i].x - controls[i].mx;
		dy = object[i].y - controls[i].my;
		if (dx == 0)
		  dx = 0.001;
		a = atan (fabs (dy) / fabs (dx));
		if (dx < 0 && dy >= 0)
		  object[i].rotation = a + RAD (90);
		else if (dx < 0 && dy < 0)
		  object[i].rotation = RAD (90) - a;
		else if (dx >= 0 && dy < 0)
		  object[i].rotation = a + RAD (270);
		else if (dx >= 0 && dy >= 0)
		  object[i].rotation = RAD (270) - a;
		if (controls[i].mask)
		  accel (i ,1.0);
	      }
	      break;
	    case C_RKEYBOARD:
	      if (controls[i].mask & 1)
		object[i].rotation += ROTSTEP;
	      if (controls[i].mask & 2)
		object[i].rotation -= ROTSTEP;
	      if (controls[i].mask & 4)
		accel (i ,1.0);
	      break;
	    case C_KEYBOARD:
	      switch (controls[i].mask)
		{
		case 1:
		  object[i].rotation = RAD (-135), accel (i ,1.0);
		  break;
		case 2:
		  object[i].rotation = RAD (135), accel (i ,1.0);
		  break;
		case 3:
		  object[i].rotation = RAD (45), accel (i ,1.0);
		  break;
		case 4:
		  object[i].rotation = RAD (-45), accel (i ,1.0);
		  break;
		case 5:
		  object[i].rotation = RAD (-90), accel (i ,1.0);
		  break;
		case 6:
		  object[i].rotation = RAD (90), accel (i ,1.0);
		  break;
		case 7:
		  object[i].rotation = RAD (180), accel (i ,1.0);
		  break;
		case 8:
		  object[i].rotation = RAD (0), accel (i ,1.0);
		  break;
		}

	    }
	}
    }
}

void
process_keys ()
{
  int             i;
  static int lasth=0;
#ifdef JOYSTICK
  int             status;
  struct JS_DATA_TYPE js;
#endif


  UpdateInput ();
  if (IsPressedH () && !lasth)
    {
      helpmode^=1;
    }
  lasth=IsPressedH();
  if (IsPressedP () && !client)
    {
      int             k = 1;
      SetScreen (backscreen);
      DrawText (MAPWIDTH / 2 - 20, MAPHEIGHT / 2 - 4, "PAUSE");
      CopyToScreen(backscreen);
#ifdef OS2DIVE
      forceBlitting ();
#endif
      tbreak = 1;
      while (k)
	{
	  UpdateInput ();
	  k = Pressed ();
#ifdef OS2DIVE
	  DosSleep (WAIT);
#endif
	}
      while (!k)
	{
	  UpdateInput ();
	  k = Pressed ();
#ifdef OS2DIVE
	  DosSleep (WAIT);
#endif
	}
    }
  switch (gamemode)
    {
    case MENU:
      menu_keys ();
      break;
    case KEYS:
      keys_keys ();
      break;
#ifdef JOYSTICK
    case JOY:
      joy_keys ();
      break;
#endif
    case GAME:
#ifdef JOYSTICK
      for (i = 0; i < 2; i++)
	{
	  double          x, y;
	  if (joystickplayer[i] >= 0)
	    {
	      if (object[joystickplayer[i]].type != ROCKET)
		continue;
	      status = read (joystickdevice[i], &js, JS_RETURN);
	      if (status != JS_RETURN)
		{
		  break;
		}
	      x = center[i][0] - js.x;
	      y = center[i][1] - js.y;	       
              if (x == 0)
		x = 0.001;
	      controls[joystickplayer[i]].jx = x;
	      controls[joystickplayer[i]].jy = y;
	      controls[joystickplayer[i]].mask = js.buttons;
	      controls[joystickplayer[i]].type = C_JOYSTICK1;

	    }
	}
#endif
#ifdef MOUSE
      /* Move. */
      if (mouseplayer != -1 && object[mouseplayer].type == ROCKET
      /*&& (MouseButtons ()||controls[mouseplayer].mask) */ )
	{
	  controls[mouseplayer].mx = MouseX () * DIV;
	  controls[mouseplayer].my = MouseY () * DIV;
	  controls[mouseplayer].mask = MouseButtons () != 0;
	  controls[mouseplayer].type = C_MOUSE;
	}
#endif
      if (IsPressedEsc ())
	{
#ifdef NETSUPPORT
	  if (!client)
	    {
#endif
	      gamemode = MENU;
	      while (IsPressedEsc ())
		UpdateInput ();
#ifdef NETSUPPORT
	    }
	  else
	    {
	      CQuit ("client exit-ESC pressed\n");
	    }
#endif
	}
      for (i = 0; i < nrockets; i++)
	{
#ifdef MOUSE
	  if (i == mouseplayer)
	    continue;
#endif
#ifdef JOYSTICK
	  if (i == joystickplayer[0] ||
	      i == joystickplayer[1])
	    continue;
#endif
#ifdef NETSUPPORT
	  if (client && !control[i])
	    continue;
#endif
	  if (object[i].type != ROCKET)
	    continue;
	  if (rotation[i])
	    {
	      char            s = 0;
	      if (IsPressed (keys[i][1]))
		s = 1;
	      if (IsPressed (keys[i][2]))
		s |= 2;
	      if (IsPressed (keys[i][0]))
		s |= 4;
	      controls[i].type = C_RKEYBOARD;
	      controls[i].mask = s;
	    }
	  else
	    {
	      int             s = 0;
	      if (IsPressed (keys[i][2]) && IsPressed (keys[i][0]))
		s = 1;
	      else if (IsPressed (keys[i][3]) && IsPressed (keys[i][0]))
		s = 2;
	      else if (IsPressed (keys[i][1]) && IsPressed (keys[i][3]))
		s = 3;
	      else if (IsPressed (keys[i][1]) && IsPressed (keys[i][2]))
		s = 4;
	      else if (IsPressed (keys[i][2]))
		s = 5;
	      else if (IsPressed (keys[i][3]))
		s = 6;
	      else if (IsPressed (keys[i][0]))
		s = 7;
	      else if (IsPressed (keys[i][1]))
		s = 8;
	      controls[i].type = C_KEYBOARD;
	      controls[i].mask = s;
	    }
	}
      break;
    }



}

#define MIN(a,b) ((a)>(b)?(b):(a))
/*
 * Make creations happen as coalescing circular cloud.  Do this by
 * creating random points within circle defined from center of screen, and
 * giving them velocity towards desired final point.
 */

void
creators_points (int radius, int x1, int y1, int color1)
{
    int             z, x, y, x2, y2;
    double r;
    int             time = 50;
    int             midX, midY, r2,r1;

    midX = GAMEWIDTH / 2;
    midY = GAMEHEIGHT / 2;
    r2 = r1 = MIN(midX, midY);
    r2 *= r2;

    z = radius * radius * M_PI / DIV / DIV;
    while (z--) {
	do {
	    x = rand() % GAMEWIDTH;
	    y = rand() % GAMEHEIGHT;
	} while (((x-midX)*(x-midX) + (y-midY)*(y-midY)) > r2);
	r=sqrt((double)((x-midX)*(x-midX) + (y-midY)*(y-midY)));
	r=(r*radius/r1)/r*0.9;
	x2=x1+(x-midX)*r;
	y2=y1+(y-midY)*r;

	addpoint(x * 256, y * 256,
		    (x2 - x) * 256 / (time),
		    (y2 - y) * 256 / (time),
		    color1 + (rand() % (DIV == 1 ? 16 : 32)),
		    time);
    }
}

void
creator (CONST int type)
{
  int             i;
  int             color1 = color (type, 0, 0);
  for (i = nrockets; i < nobjects && (object[i].live ||
				      object[i].type == CREATOR);
       i++);
  if (i >= MAXOBJECT)
    return;
  if (!find_possition (&object[i].x, &object[i].y, radius (type)))
    return;
  if (i >= nobjects)
    nobjects = i + 1;
  object[i].live = 0;
  object[i].live1 = 1;
  object[i].lineto = -1;
  object[i].ctype = type;
  object[i].fx = 0.0;
  object[i].fy = 0.0;
  object[i].time = 50;
  object[i].rotation = 0;
  object[i].type = CREATOR;
  object[i].M = M (type);
  object[i].radius = radius (type);
  object[i].accel = ROCKET_SPEED;
  object[i].letter = ' ';
#ifdef NETSUPPORT
  if (server)
    CreatorsPoints (object[i].radius, object[i].x, object[i].y, color1);
  else
#endif
    creators_points (object[i].radius, object[i].x, object[i].y, color1);
  Effect (S_CREATOR1, 0);
}


void
creator_rocket (CONST int i)
{
  int             type = ROCKET;
  int             color1 = color (ROCKET, i, 0);
  if (!find_possition (&object[i].x, &object[i].y, radius (type)))
    return;
  if (sound)
    object[i].live1 = object[i].live;
  object[i].live = 0;
  object[i].thief = 0;
  object[i].ctype = type;
  object[i].lineto = -1;
  object[i].fx = 0.0;
  object[i].fy = 0.0;
  object[i].time = 50;
  object[i].rotation = 0;
  object[i].type = CREATOR;
  object[i].M = ROCKETM;
  object[i].radius = ROCKET_RADIUS;
  object[i].accel = ROCKET_SPEED;
  object[i].letter = ' ';
#ifdef NETSUPPORT
  if (server)
    CreatorsPoints (ROCKET_RADIUS, object[i].x, object[i].y, color1);
  else
#endif
    creators_points (ROCKET_RADIUS, object[i].x, object[i].y, color1);
}




static void
update_forces ()
{
  int             i;
  int             r;
  float           d;
  float           xp, yp;
  int             frocket = 0;
  for (i = 0; i < nobjects; i++)
    {
      if (object[i].live)
	{
	  if (object[i].lineto != -1)
	    {
	      if (!object[object[i].lineto].live)
		object[i].lineto = -1;
	      else if (object[i].lineto == i)
		object[i].lineto = -1;
	      else
		{
		  int             force;
		  xp = object[i].x - object[object[i].lineto].x;
		  yp = object[i].y - object[object[i].lineto].y;
		  force = sqrt (xp * xp + yp * yp);
		  if (force >= 2 * SPRINGSIZE || gameplan == COOPERATIVE)
		    {
		      force = force - SPRINGSIZE;
		      if (force < 0)
			force *= 3;
		      force = force / SPRINGSTRENGTH;
		      normalize (&xp, &yp, force * BALL_SPEED / object[i].M);
		      object[i].fx -= xp;
		      object[i].fy -= yp;
		      normalize (&xp, &yp, force * BALL_SPEED / object[object[i].lineto].M);
		      object[object[i].lineto].fx += xp;
		      object[object[i].lineto].fy += yp;
		    }
		}
	    }
	  if (object[i].type == ROCKET && object[i].time)
	    object[i].time--;
	  if (object[i].type == ROCKET && !object[i].time)
	    {
	      d = 640 * 640;
	      frocket = -1;
	      for (r = 0; r < nobjects; r++)
		{
		  if (object[r].live && !object[r].time && object[r].type == EHOLE)
		    {
		      int             distance;
		      float           gravity;
		      xp = object[r].x - object[i].x;
		      yp = object[r].y - object[i].y;
		      distance = sqrt (xp * xp + yp * yp);
		      gravity = BALL_SPEED * (gameplan == COOPERATIVE ? 200 : 50) / distance;
		      if (gravity > BALL_SPEED * 4 / 5)
			gravity = BALL_SPEED * 4 / 5;
		      normalize (&xp, &yp, gravity);
		      object[i].fx += xp;
		      object[i].fy += yp;
		    }
		}

	    }
	  if (object[i].type == BALL || object[i].type == LBALL || object[i].type == BBALL || object[i].type == LUNATIC)
	    {
	      frocket = -1;
	      d = 640 * 640;
	      for (r = 0; r < nrockets; r++)
		{
		  if (object[r].live && !object[r].time)
		    {
		      xp = object[r].x - object[i].x;
		      yp = object[r].y - object[i].y;
		      if (xp * xp + yp * yp < d)
			d = xp * xp + yp * yp, frocket = r;
		    }
		}
	      if (frocket != -1)
		xp = object[frocket].x - object[i].x,
		  yp = object[frocket].y - object[i].y;
	      else
		xp = GAMEWIDTH / 2 - object[i].x,
		  yp = GAMEHEIGHT / 2 - object[i].y;
	      if (object[i].type == LUNATIC && !rand () % 4)
		{
		  xp = rand ();
		  yp = rand () + 1;
		}
	      switch (object[i].type)
		{
		case BBALL:
		  normalize (&xp, &yp, BBALL_SPEED);
		  break;
		case BALL:
		case LUNATIC:
		case LBALL:
		  normalize (&xp, &yp, BALL_SPEED);
		  break;
		}
	      object[i].fx += xp;
	      object[i].fy += yp;
	    }
	  object[i].fx *= SLOWDOWN,
	    object[i].fy *= SLOWDOWN;
	}
    }
}




static void
colisions ()
{
  int             i, y;
  int             colize = 0;
  static int      ctime = 0;
  float           xp, yp, gummfactor;
  for (i = 0; i < nobjects; i++)
    if (object[i].live)
      for (y = i + 1; y < nobjects; y++)
	if (object[y].live)
	  {
	    xp = object[y].x - object[i].x;
	    yp = object[y].y - object[i].y;
	    if (xp * xp + yp * yp < (object[y].radius + object[i].radius) *
		(object[y].radius + object[i].radius))
	      {
		colize = 1;
		if (object[i].type == HOLE || object[i].type == EHOLE)
		  {
		    if (object[y].type != APPLE)
		      destroy (y);
		    if (object[i].type == EHOLE)
		      destroy (i);
		    continue;
		  }
		if (object[y].type == HOLE || object[y].type == EHOLE)
		  {
		    if (object[i].type != APPLE)
		      destroy (i);
		    if (object[y].type == EHOLE)
		      destroy (y);
		    continue;
		  }
		if (object[i].type == ROCKET)
		  {
		    if (object[y].thief == 1 && object[i].thief == 1)
		      {
			float           tmp;
			tmp = object[i].M;
			object[i].M = object[y].M;
			object[y].M = tmp;
			object[i].thief = 0;
			object[y].thief = 0;
		      }
		    if (object[y].type == BBALL && object[i].thief == 1)
		      {
			object[i].M += object[y].M - M (BALL);
			object[i].thief = 0;
			object[y].M = M (BALL);
		      }
		    else if (object[y].type == ROCKET && object[i].thief == 1)
		      {
			object[i].M += object[y].M - M (ROCKET);
			object[i].accel += object[y].accel - ROCKET_SPEED;
			object[i].thief = 0;
			object[y].M = M (object[i].type);
			object[y].accel = ROCKET_SPEED - A_ADD;
		      }
		    if (object[i].type == ROCKET && object[y].thief == 1)
		      {
			object[y].M += object[i].M - M (ROCKET);
			object[y].accel += object[i].accel - ROCKET_SPEED;
			object[y].thief = 0;
			object[i].M = M (object[y].type);
			object[i].accel = ROCKET_SPEED - A_ADD;
		      }
		    if (gameplan == COOPERATIVE)
		      object[i].score++;
		    if (object[y].letter == L_ACCEL)
		      object[i].accel += A_ADD,
			object[i].score += 10;
		    if (object[y].letter == L_GUMM)
		      object[i].M += M_ADD,
			object[i].score += 10;
		    if (object[y].letter == L_THIEF)
		      object[i].M = M (object[i].type),
			object[i].accel = ROCKET_SPEED - A_ADD,
			object[i].score -= 30;
		    if (object[y].letter == L_FINDER)
		      {
			object[i].accel += A_ADD * (rand () % 5);
			object[i].M += M_ADD * (rand () % 10);
			object[i].score += 30;
		      }
		    if (object[y].letter == L_TTOOL)
		      {
			object[i].thief = 1;
			object[i].score += 30;
		      }

		    object[y].letter = ' ';
		    if (object[y].type == LBALL)
		      object[y].type = BALL;
		    if (object[y].type == BALL && dosprings && !(rand () % randsprings))
		      object[y].lineto = i;

		    if (gameplan == DEATHMATCH && object[y].type == ROCKET && dosprings && !(rand () % (2 * randsprings)))
		      object[y].lineto = i;
		  }
		if (object[y].type == LUNATIC)
		  {
		    gummfactor = -ROCKETM / LUNATICM;
		  }
		else if (object[i].type == LUNATIC)
		  {
		    gummfactor = -LUNATICM / ROCKETM;
		  }
		else
		  gummfactor = object[i].M / object[y].M;
		normalize (&xp, &yp, gummfactor * GUMM);
		object[y].fx += xp;
		object[y].fy += yp;
		normalize (&xp, &yp, 1 / gummfactor * GUMM);
		object[i].fx -= xp;
		object[i].fy -= yp;
		if (object[i].type == ROCKET && object[i].time)
		  object[i].fx = 0,
		    object[i].fy = 0;
		if (object[y].type == ROCKET && object[y].time)
		  object[y].fx = 0,
		    object[y].fy = 0;
		if (object[y].type == INSPECTOR && object[i].type == ROCKET)
		  {
		    object[y].fx = 0,
		      object[y].fy = 0;
		    object[i].fx *= -2,
		      object[i].fy *= -2;
		  }
	      }
	  }
  if (colize && !ctime)
    {
#ifndef NAS_SOUND
      Effect (S_COLIZE, next);
#endif
      ctime = 4;
    }
  if (ctime)
    ctime--;
}



void
game ()
{
  long            VfTime = 0;
  long            VendSleep = 0;
  struct timeval  VlastClk;
  struct timeval  VnewClk;
  int             wait = 0;

  load_rc ();
  init_menu ();
  gettimeofday (&VlastClk, NULL);
  gettimeofday (&VnewClk, NULL);
  VendSleep = VlastClk.tv_usec;
  VfTime = 1000000 / 25;


  while (1)
    {
      process_keys ();
      sprocess_keys ();
      update_values ();
      update_game ();
      update_forces ();
      colisions ();
      move_objects ();
      check_limit ();
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      if (wait > 0 || tbreak)
	draw_objects (1);
      else
	draw_objects (0);
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      if (tbreak)
	wait = VfTime;
      if (wait > 0)
	usleep (wait);
      VendSleep = VnewClk.tv_usec + wait;
      gettimeofday (&VlastClk, NULL);
      if (tbreak)
	tbreak = 0,
	  VendSleep = VlastClk.tv_usec;

    }
}
#ifdef NETSUPPORT
void
client_loop2 (int draw)		/*game part of server loop */
{
  draw_objects (draw);
  switch (gamemode)
    {
    case MENU:
      draw_menu (draw);
      break;
    case KEYS:
      draw_keys (draw);
      break;
#ifdef JOYSTICK
    case JOY:
      draw_joy (draw);
      break;
#endif
    }
}
void
server_loop2 (void)		/*game part of server loop */
{
  sprocess_keys ();
  update_values ();
  update_game ();
  update_forces ();
  colisions ();
  move_objects ();
  check_limit ();
}
#endif
