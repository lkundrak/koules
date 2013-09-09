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
*    Copyright(c)1995,1996 by Jan Hubicka.See README for   *
*                    licence details.                      *
*----------------------------------------------------------*
*  intro.c intro outro code                                *
***********************************************************/
/* Changes for OS/2 Warp with Dive.                        *
 *  Copyright(c)1996 by Thomas A. K. Kjaer                 *
 ***********************************************************/

#include "koules.h"
#include "font.h"
#include "text.h"
#include <sys/time.h>
#include <unistd.h>
extern void     fadein1 ();
extern void     points ();
extern void     points1 ();


/*

 *  intro code
 *
 */
void
draw_koules (CONST int c, CONST int s, CONST int r)
{
  int             i;
  for (i = 0; i < 360; i += 60)
    switch (c)
      {
      case 0:
	PutBitmap ((int) (MAPWIDTH / 2 - BALL_RADIUS / DIV + sin (RAD (i + s)) * r),
	  (int) (MAPHEIGHT / 2 - BALL_RADIUS / DIV + cos (RAD (i + s)) * r),
		 BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, ball_bitmap);
	break;
      case 1:
	PutBitmap ((int) (MAPWIDTH / 2 - BALL_RADIUS / DIV + sin (RAD (i + s)) * r),
	  (int) (MAPHEIGHT / 2 - BALL_RADIUS / DIV + cos (RAD (i + s)) * r),
	     BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, lball_bitmap[0]);
	break;
      default:
	PutBitmap ((int) (MAPWIDTH / 2 - BALL_RADIUS / DIV + sin (RAD (i + s)) * r),
	  (int) (MAPHEIGHT / 2 - BALL_RADIUS / DIV + cos (RAD (i + s)) * r),
	     BALL_RADIUS * 2 / DIV, BALL_RADIUS * 2 / DIV, lball_bitmap[1]);
	break;
      }

}
void
koulescreator (CONST int r)
{
  int             time = 100;
  int             z1;
  int             x, y;
  int             x1, y1, i;
  Effect (S_CREATOR1, next);
  for (i = 0; i < 360; i += 60)
    {
      x1 = (MAPWIDTH / 2 - +sin (RAD (i)) * r);
      y1 = (MAPHEIGHT / 2 - +cos (RAD (i)) * r);
      for (z1 = 0; z1 < BALL_RADIUS * BALL_RADIUS * M_PI / DIV / DIV; z1++)
	{
	  x = rand () % GAMEWIDTH;
	  y = rand () % GAMEHEIGHT;
	  addpoint (x * 256 * DIV, y * 256 * DIV,
		    (x1 - x) * 256 / (time) * DIV,
		    (y1 - y) * 256 / (time) * DIV,
		    ball (rand () % 32),
		    time);
	}
    }
}
void
starcreator ()
{
  int             time = 100;
  int             z;
  int             x, y;
  Effect (S_CREATOR1, next);
  for (z = 0; z < ROCKET_RADIUS * ROCKET_RADIUS * M_PI / DIV / DIV; z++)
    {
      x = rand () % GAMEWIDTH;
      y = rand () % GAMEHEIGHT;
      addpoint (x * 256 * DIV, y * 256 * DIV,
		(MAPWIDTH / 2 - x) * 256 / (time) * DIV,
		(MAPHEIGHT / 2 - y) * 256 / (time) * DIV,
		(rand () % 32),
		time);
    }
}
void
draw_player (CONST int x, CONST int y, CONST float r, CONST int z, CONST int r1)
{
  int             x1, y1;
  PutBitmap ((int) x - ROCKET_RADIUS / DIV, (int) y - ROCKET_RADIUS / DIV,
	ROCKET_RADIUS * 2 / DIV, ROCKET_RADIUS * 2 / DIV, rocket_bitmap[z]);
  x1 = x * DIV + sin (r - RAD (30)) * r1 - EYE_RADIUS;
  y1 = y * DIV + cos (r - RAD (30)) * r1 - EYE_RADIUS;
  PutBitmap (x1 / DIV, y1 / DIV,
	     EYE_RADIUS * 2 / DIV, EYE_RADIUS * 2 / DIV, eye_bitmap[z]);
  x1 = x * DIV + sin (r + RAD (30)) * r1 - EYE_RADIUS;
  y1 = y * DIV + cos (r + RAD (30)) * r1 - EYE_RADIUS;
  PutBitmap (x1 / DIV, y1 / DIV,
	     EYE_RADIUS * 2 / DIV, EYE_RADIUS * 2 / DIV, eye_bitmap[z]);

}

#define TEXTW 200
#define TEXTH 25
void
starwars ()
{
  int             y, i, z;
  float           r[3];
  float           angle = 0;

  int             actu = 0;
  int             time = 0;
  int             time1 = 0;

  int             playx = 0, playy = 0;
  int             bballx = 0, bbally = 0;
  float           playr = RAD (0), playp = 0.03;
  long            VfTime = 0;
  long            VendSleep = 0;
  int             sizes[300];
  struct timeval  VlastClk;
  struct timeval  VnewClk;
  int             wait = 0;

  float           r1;
  float           rp[3];

  fadeout ();
  for (y = 0; y < TEXTSIZE; y++)
    sizes[y] = -vgatextsize (TEXTH, text[y]) / 2;
  gettimeofday (&VlastClk, NULL);
  gettimeofday (&VnewClk, NULL);
  VendSleep = VlastClk.tv_usec;
#ifdef DEBUG
  VfTime = 1000000 / 200;
#else
  VfTime = 1000000 / 65;
#endif

  r[1] = r[2] = r[0] = sqrt ((MAPWIDTH / 2) * (MAPWIDTH / 2) +
			     (MAPHEIGHT / 2) * (MAPHEIGHT / 2));
  rp[0] = 0.0 / DIV;
  rp[1] = 0.6 / DIV;
  rp[2] = 0.6 / DIV;
  r1 = r[0];

  Effect (S_START, 0);
  for (i = -660; i < (TEXTSIZE + 10) * TEXTW; i += 1)
    {
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      if (wait >= 0 || tbreak)
	{
	  CopyVSToVS (starbackground, backscreen);
	  SetScreen (backscreen);
	  EnableClipping ();
	  for (z = 0; z < 3; z++)
	    {
	      if (r[z] <= ROCKET_RADIUS / DIV + BALL_RADIUS / DIV)
		{
		  rp[z] = -6 / DIV;
		  Effect (S_COLIZE, next);
		}
	      if (r[z] < r1)
		draw_koules (rp[z] > 0 ? z : 0, (int) angle, r[z]);
	    }
	  if (bbally > -20 && bballx)
	    PutBitmap ((int) bballx - (BBALL_RADIUS) / DIV, bbally,
	      BBALL_RADIUS * 2 / DIV, BBALL_RADIUS * 2 / DIV, bball_bitmap);

	  points ();
	  for (y = 0; y < TEXTSIZE; y++)
	    {
	      if (y * TEXTW - i + 2 * TEXTW < 1000 && y * TEXTW - i + TEXTW > -1500)
		{
		  textcolor = (1200 + (y * TEXTW - i)) * 32 / 2500;
		  if (textcolor <= 0)
		    continue;
		  actu = y;
		  vgadrawtext (sizes[y], y * TEXTW - i, TEXTH, text[y]);
#ifdef OS2DIVE
		  DosSleep (WAIT);
#endif
		}
	    }
	  if (playx)
	    draw_player (playx, playy, playr, 0, EYE_RADIUS1);
	  if (actu == PLAYERLINE && !time)
	    starcreator (), time = 1;
	  CopyToScreen (backscreen);
	  fadein1 ();
	}
      else
	points1 ();


      if (actu >= KOULESLINE && !time1)
	koulescreator (MAPHEIGHT / 2 - 20), time1 = 1;
      if (time1)
	time1++;
      if (time1 == 100)
	r[0] = MAPHEIGHT / 2 - 20;
      if (time1 > 100)
	r[0] -= rp[0], angle += 0.3;


      playr += playp;
      if (playr < RAD (-45))
	playp = 0.015, playr = RAD (-45);
      if (playr > RAD (45))
	playp = -0.03, playr = RAD (45);
      if (actu >= D1LINE)
	r[1] -= rp[1];
      if (actu >= D2LINE)
	r[2] -= rp[2];
      if (actu >= BLINE && !bballx)
	bballx = MAPWIDTH / 2,
	  bbally = MAPHEIGHT + 30;
      if (bballx)
	bbally--;
      if (bbally > 0 && bbally < MAPHEIGHT / 2 + ROCKET_RADIUS / 2)
	{
	  if (playy == MAPHEIGHT / 2)
	    {
	      Effect (S_END, next);
	    }
	  playy -= 10;
	}
      if (time)
	time++;
      if (time == 100)
	{
	  playx = MAPWIDTH / 2, playy = MAPHEIGHT / 2, playr = RAD (180), rp[0] = 1.5 / DIV;
	  Effect (S_CREATOR2, next);
	}
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      if (tbreak)
	wait = VfTime, tbreak = 0;
      usleep (wait < 0 ? 0 : wait);
      VendSleep = VnewClk.tv_usec + wait;
      gettimeofday (&VlastClk, NULL);
      UpdateInput ();
      if (Pressed ())
	{
	  fadeout ();
	  while (Pressed ())
	    {
	      UpdateInput ();
	    }
	  return;
	}
    }
  fadeout ();
}
void
clearpoints ()
{
  int             i;
  for (i = 0; i < MAXPOINT; i++)
    point[i].time = 0;
}
void
outro (CONST int size, char *text[])
{
  int             y, i;

  int             actu = -1;
  int             lkey = 1;
  int             skey = 0;
  int             key = 1;

  long            VfTime = 0;
  long            VendSleep = 0;
#ifdef __GNUC__
  int             sizes[size];
#else
  int             sizes[300];
#endif
  struct timeval  VlastClk;
  struct timeval  VnewClk;
  int             wait = 0;

  fadeout ();
  clearpoints ();
  for (y = 0; y < size; y++)
    sizes[y] = -vgatextsize (TEXTH, text[y]) / 2;
  gettimeofday (&VlastClk, NULL);
  gettimeofday (&VnewClk, NULL);
  VendSleep = VlastClk.tv_usec;
#ifdef DEBUG
  VfTime = 1000000 / 200;
#else
  VfTime = 1000000 / 65;
#endif

  for (i = -660; i < (size + 4) * TEXTW; i += 1)
    {
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      if (wait >= 0 || tbreak)
	{
	  CopyVSToVS (starbackground, backscreen);
	  SetScreen (backscreen);
	  EnableClipping ();
	  points ();
	  for (y = 0; y < size; y++)
	    {
	      if (y * TEXTW - i + 2 * TEXTW < 1000 && y * TEXTW - i + TEXTW > -1500)
		{
		  textcolor = (1200 + (y * TEXTW - i)) * 32 / 2500;
		  if (textcolor <= 0)
		    continue;
		  actu = y;
		  vgadrawtext (sizes[y], y * TEXTW - i, TEXTH, text[y]);
#ifdef OS2DIVE
		  DosSleep (WAIT);
#endif
		}
	    }
	  CopyToScreen (backscreen);
	  fadein1 ();
	}
      else
	points1 ();


      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      if (tbreak)
	wait = VfTime, tbreak = 0;
      usleep (wait < 0 ? 0 : wait);
      VendSleep = VnewClk.tv_usec + wait;
      gettimeofday (&VlastClk, NULL);
      UpdateInput ();
      if (actu >= 0)
	{
	  lkey = key;
	  key = Pressed ();
	  if (skey && !key && lkey)
	    {
	      fadeout ();
	      return;
	    }
	  if (!key && lkey)
	    skey = 1;
	}
    }
  fadeout ();
}
void
staraccel (CONST float x1, CONST float y1, CONST float r)
{
  int             y;
  for (y = 0; y < 5 / DIV / DIV; y++)
    {
      float           p;
      p = RAD (rand () % 45 - 22);
      addpoint (x1 * 256,
		y1 * 256,
		(-sin (r + p) * 0.08 * 10) * (rand () % 512),
		(-cos (r + p) * 0.08 * 10) * (rand () % 512),
		rocket (rand () % 16), 30);
    }
}
void
outro2 ()
{
  int             y, i, z;
  int             lkey = 1;
  int             skey = 0;
  int             key = 1;

  int             actu = 0;
  float           width = 0;

  long            VfTime = 0;
  long            VendSleep = 0;
  int             size = TEXTSIZE2;
#ifdef __GNUC__
  int             sizes[size], nrockets1;
#else
  int             sizes[300], nrockets1;
#endif
  struct timeval  VlastClk;
  struct timeval  VnewClk;
  int             wait = 0;
  int		 first=1;
  float           playy = MAPHEIGHT + 20;
  float           playr = RAD (180);
  float           playp = 0;
  float           er = EYE_RADIUS1;
  float           erp = 2;


  fadeout ();
  clearpoints ();
  nrockets1 = nrockets - 1;
  if (nrockets1 == 0)
    nrockets1 = 1;
  for (y = 0; y < size; y++)
    sizes[y] = -vgatextsize (TEXTH, text2[y]) / 2;
  gettimeofday (&VlastClk, NULL);
  gettimeofday (&VnewClk, NULL);
  VendSleep = VlastClk.tv_usec;
#ifdef DEBUG
  VfTime = 1000000 / 200;
#else
  VfTime = 1000000 / 65;
#endif
  Effect (S_START, next);
  for (i = -660; i < (size + 5) * TEXTW; i += 1)
    {
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      if (wait >= 0 || tbreak)
	{
	  CopyVSToVS (starbackground, backscreen);
	  SetScreen (backscreen);
	  EnableClipping ();
	  points ();
	  for (y = 0; y < size; y++)
	    {
	      if (y * TEXTW - i + 2 * TEXTW < 1000 && y * TEXTW - i + TEXTW > -1500)
		{
		  textcolor = (1200 + (y * TEXTW - i)) * 32 / 2500;
		  if (textcolor <= 0)
		    continue;
		  actu = y;
		  vgadrawtext (sizes[y], y * TEXTW - i, TEXTH, text2[y]);
#ifdef OS2DIVE
		  DosSleep (WAIT);
#endif
		}
	    }
	  for (z = 0; z < nrockets; z++)
	    {
	      if(!first) draw_player (MAPWIDTH / 2 - width / 2 + z * width / (nrockets1), playy, playr, z, (int) er);
	    }
	  first=0;
	  CopyToScreen (backscreen);
	  fadein1 ();
	}
      else
	points1 ();
      width = (200 * (nrockets - 1) * playy / MAPHEIGHT + 20 * (nrockets - 1)) / DIV;
      if (playy > MAPHEIGHT / 2 || actu > CONTLINE)
	{
	  playy -= 0.6 / DIV;
	  playr = RAD (180);
	  for (z = 0; z < nrockets; z++)
	    {
	      staraccel ((MAPWIDTH / 2 - width / 2 + z * width / (nrockets1)) * DIV, (playy) * DIV, playr);
	    }
	  er = EYE_RADIUS1;
	}
      else
	{
	  playr += playp;
	  if (playr < RAD (-45))
	    playp = 0.015, playr = RAD (-45);
	  if (playr > RAD (45))
	    playp = -0.03, playr = RAD (45);
	  if (actu > UDIVLINE)
	    {
	      /*if (er < 2 * EYE_RADIUS1)
	         erp += 0.1;
	         if (er > 2 * EYE_RADIUS1)
	         erp -= 0.1; */
	      erp += (2 * EYE_RADIUS1 - er) / 10;
	      erp *= 0.98;
	      er += erp;
	    }
	}
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      if (tbreak)
	wait = VfTime, tbreak = 0;
      usleep (wait < 0 ? 0 : wait);
      VendSleep = VnewClk.tv_usec + wait;
      gettimeofday (&VlastClk, NULL);
      UpdateInput ();
      if (actu > 0)
	{
	  lkey = key;
	  key = 0;
	  key = Pressed ();
	  if (skey && !key && lkey)
	    {
	      fadeout ();
	      return;
	    }
	  if (!key && lkey)
	    skey = 1;
	}
    }
  fadeout ();
}


void
outro1 ()
{
  outro (TEXTSIZE1, text1);
}

void
intro_intro ()
{
  outro (INTROSIZE, introtext);
}

void
hole_intro ()
{
  outro (HOLESIZE, holetext);
}

void
inspector_intro ()
{
  outro (INSPECTORSIZE, inspectortext);
}

void
bball_intro ()
{
  outro (BBALLSIZE, bballtext);
}

void
bbball_intro ()
{
  outro (BBBALLSIZE, bbballtext);
}

void
maghole_intro ()
{
  outro (MAGSIZE, magholetext);
}
void
spring_intro ()
{
  outro (SPRINGTSIZE, springtext);
}
void
thief_intro ()
{
  outro (THIEFSIZE, thieftext);
}
void
ttool_intro ()
{
  outro (TTOOLSIZE, ttooltext);
}
void
finder_intro ()
{
  outro (FINDERSIZE, findertext);
}
void
lunatic_intro ()
{
  outro (LUNATICSIZE, lunatictext);
}
