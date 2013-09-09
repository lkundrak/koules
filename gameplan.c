/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C1995 JAHUSOFT                                          *
*        Jan Hubicka                                       *
*        Dukelskych bojovniku 1944                         *
*        390 03 Tabor                                      *
*        Czech Republic                                    *
*        Phone: 0041-361-32613                             *
*        eMail: hubicka@limax.paru.cas.cz                  *
*----------------------------------------------------------*
*    Copyright(c)1995,1996 by Jan Hubicka.See README for   *
*                    licence details.                      *
*----------------------------------------------------------*
*  gameplan.c scores,levels......                          *
***********************************************************/
/* Changes for OS/2 Warp with Dive.                        *
 *  Copyright(c)1996 by Thomas A. K. Kjaer                 *
 ***********************************************************/

#include "koules.h"
#include "server.h"
#define HOLELEVEL 5
#define BBALLLEVEL (nrockets==1?12:10)
#define EHOLELEVEL 20
#define SPRINGLEVEL 30
#define THIEFLEVEL 40
#define FINDERLEVEL 50
#define TTOOLLEVEL 60
#define INSPECTORLEVEL 70
#define BBBALLLEVEL 90
#define LUNATICLEVEL 80

static int      level = 0;
static int      gtime;
static int      nos;
static int      ktime = 0;
static int      kmode = 0;
static int      secondpart;
void
gameplan_init ()
{
#ifdef DEBUG
  level = 98;
#else
  level = lastlevel;
#endif
}
void
effect (int so)
{
  char            s[20];
  if (gamemode == GAME)
    {
      if (!so)
	Effect (S_START, 0);
      else
	Effect (S_END, 0);
    }
  if (gameplan == COOPERATIVE)
    {
#ifdef SOUND
#endif
      gamemode = MENU;
#ifndef DEBUG
      if (!so && level == 0)
#endif
	intro_intro ();
#ifndef DEBUG
      if (!so && level == HOLELEVEL + 1)
#endif
	hole_intro ();
#ifndef DEBUG
      if (!so && level == INSPECTORLEVEL)
#endif
	inspector_intro ();
#ifndef DEBUG
      if (!so && level == BBALLLEVEL + 1)
#endif
	bball_intro ();
#ifndef DEBUG
      if (!so && level == BBBALLLEVEL)
#endif
	bbball_intro ();
#ifndef DEBUG
      if (!so && level == EHOLELEVEL)
#endif
	maghole_intro ();
#ifndef DEBUG
      if (!so && level == SPRINGLEVEL + 1)
#endif
	spring_intro ();
#ifndef DEBUG
      if (!so && level == THIEFLEVEL)
#endif
	thief_intro ();
#ifndef DEBUG
      if (!so && level == FINDERLEVEL)
#endif
	finder_intro ();
#ifndef DEBUG
      if (!so && level == TTOOLLEVEL)
#endif
	ttool_intro ();
#ifndef DEBUG
      if (!so && level == LUNATICLEVEL)
#endif
	lunatic_intro ();
      if (!so && level == 99)
	outro1 ();
    }
  gamemode = GAME;
  SetScreen (backscreen);
  ClearScreen ();
  tbreak = 1;
  if (gameplan == COOPERATIVE)
    sprintf (s, "SECTOR %i", level + 1);
  else
    sprintf (s, "GET READY");
#ifdef XSUPPORT
  DrawWhiteMaskedText (MAPWIDTH / 2 - 32, MAPHEIGHT / 2 - 4, s);
#else
  DrawText (MAPWIDTH / 2 - 32, MAPHEIGHT / 2 - 4, s);
#endif
  CopyToScreen (backscreen);
  fadein ();
#ifdef XSUPPORT
  XFlush (dp);
#endif
#ifdef OS2DIVE
  forceBlitting ();
#endif
  fflush (stdout);
  usleep (1000000);
}
/* 
 * level creation routines
 */
static void
init_objects1 ()
{
  int             i, nholes;
  dosprings = 0;
  lastlevel = level;
  if (maxlevel < level)
    maxlevel = level;
  save_rc ();
  if (gamemode != GAME)
    {
      nobjects = 0;
      return;
    }
  switch (gameplan)
    {
    case DEATHMATCH:
      dosprings = 1;		/* enable spring in death match */
      randsprings = 40;
      if (nrockets != 1)
	{
	  nobjects = nrockets * 3 * GAMEWIDTH / 640 * GAMEHEIGHT / 460 + 3;
	  for (i = 0; i < nobjects; i++)
	    object[i].live = 0;
	  for (i = 0; i < nobjects; i++)
	    {
	      object[i].live = 1;
	      object[i].lineto = -1;
	      object[i].live1 = 1;
	      object[i].thief = 0;
	      object[i].time = (i < nrockets ? 99 : 0);
	      object[i].fx = 0;
	      object[i].fy = 0;
	      object[i].rotation = 0;
	      object[i].type = (i < nrockets ? ROCKET : BALL);
	      if (!find_possition (&object[i].x, &object[i].y, radius (object[i].type)))
		return;
	      object[i].M = M (object[i].type);
	      object[i].radius = radius (object[i].type);
	      object[i].accel = ROCKET_SPEED;
	      object[i].letter = ' ';
	      secondpart = 0;
	    }
	  nholes = nrockets / 3 + 1;
	  for (i = 0; i < nholes; i++)
	    creator (HOLE);
	  creator (INSPECTOR);
	  creator (EHOLE);
	}
      else
	{
	  nobjects = 2;
	  for (i = 0; i < nobjects; i++)
	    object[i].live = 0;
	  for (i = 0; i < nobjects; i++)
	    {
	      object[i].live = 1;
	      object[i].lineto = -1;
	      if (i != 0)
		object[i].lineto = 0;
	      object[i].live1 = 1;
	      object[i].thief = 0;
	      object[i].time = (i < nrockets ? 99 : 0);
	      object[i].fx = 0;
	      object[i].fy = 0;
	      object[i].rotation = 0;
	      object[i].type = (i < nrockets ? ROCKET : LUNATIC);
	      if (!find_possition (&object[i].x, &object[i].y, radius (object[i].type)))
		return;
	      object[i].M = M (object[i].type);
	      object[i].radius = radius (object[i].type);
	      object[i].accel = ROCKET_SPEED;
	      object[i].letter = ' ';
	      secondpart = 0;
	    }
	}
      break;
    case COOPERATIVE:
      if (level != 99)
	{
	  int             nbballs;
	  if (level > SPRINGLEVEL)
	    dosprings = 1;
	  randsprings = 40 - level / 3;
	  nobjects = 3 + sqrt (level) * ((nrockets + 1) / 2) + 2 * nrockets;
	  nobjects = (float) nobjects *(GAMEWIDTH / 640 * GAMEHEIGHT / 460 + 2) / 3;
	  if (nobjects > 30)
	    nobjects = 30;
	  for (i = 0; i < nobjects; i++)
	    object[i].live = 0;
	  nbballs = nrockets + level / BBBALLLEVEL;
	  gtime = 100 + 1000 / (level + 1);
	  for (i = 0; i < nobjects; i++)
	    {
	      object[i].live = (i < nrockets ? 5 : 1);
	      object[i].live1 = (i < nrockets ? 5 : 1);
	      object[i].lineto = -1;
	      object[i].thief = 0;
	      object[i].time = (i < nrockets ? 100 : 0);
	      object[i].fx = 0;
	      object[i].fy = 0;
	      object[i].rotation = 0;
	      object[i].type = (i < nrockets ? ROCKET : (i < nbballs ? BBALL : BALL));
	      object[i].M = M (object[i].type);
	      if (i < nrockets)
		{
		  if (level < 5)
		    object[i].M *= 1.0 + (5.0 - level) / 15.0;
		  if (level < 25)
		    object[i].M *= 1.0 + (level) / 120.0;
		}
	      object[i].radius = radius (object[i].type);
	      object[i].accel = ROCKET_SPEED;
	      if (!find_possition (&object[i].x, &object[i].y, radius (object[i].type)))
		return;
	      object[i].letter = ' ';
	    }
	}
      else
	{
	  nobjects = nrockets + 10;
	  for (i = 0; i < nobjects; i++)
	    object[i].live = 0;
	  object[nrockets].type = APPLE;
	  object[nrockets].M = APPLEM;
	  object[nrockets].lineto = -1;
	  object[nrockets].thief = 0;
	  object[nrockets].radius = APPLE_RADIUS;
	  object[nrockets].live = (i < nrockets ? 5 : 1);
	  object[nrockets].fx = 0;
	  object[nrockets].fy = 0;
	  object[nrockets].x = GAMEWIDTH / 2;
	  object[nrockets].y = GAMEHEIGHT / 2;
	  for (i = 0; i < nrockets; i++)
	    {
	      object[i].live = 5;
	      object[i].live1 = 5;
	      object[i].time = 100;
	      object[i].thief = 0;
	      object[i].lineto = -1;
	      object[i].fx = 0;
	      object[i].fy = 0;
	      object[i].rotation = 0;
	      object[i].type = ROCKET;
	      object[i].accel = ROCKET_SPEED;
	      object[i].M = M (object[i].type);
	      object[i].radius = radius (object[i].type);
	      object[i].letter = ' ';
	      object[i].x = GAMEWIDTH / 2 + sin (i * RAD (360) / nrockets) * GAMEHEIGHT / 3;
	      object[i].x = GAMEHEIGHT / 2 + cos (i * RAD (360) / nrockets) * GAMEHEIGHT / 3;
	    }
	  for (i = nrockets + 1; i < nobjects; i++)
	    {
	      object[i].live = 1;
	      object[i].live1 = 1;
	      object[i].lineto = -1;
	      object[i].time = 0;
	      object[i].thief = 0;
	      object[i].fx = 0;
	      object[i].fy = 0;
	      object[i].rotation = 0;
	      object[i].type = BALL;
	      object[i].accel = BALL_SPEED;
	      object[i].M = M (object[i].type);
	      object[i].radius = radius (object[i].type);
	      object[i].letter = ' ';
	      if (!find_possition (&object[i].x, &object[i].y, radius (object[i].type)))
		return;
	    }
	}
    }
  ktime = 0;
}
void
init_objects ()
{
  switch (difficulty)
    {
    case 0:
      ROCKET_SPEED = 0.8;
      BALL_SPEED = 1.2;
      BBALL_SPEED = 1.2;
      SLOWDOWN = 0.9;
      GUMM = 20;


      BALLM = 3;
      LBALLM = 3;
      BBALLM = 8;
      APPLEM = 40;
      ROCKETM = 2;
      break;
    case 1:
      ROCKET_SPEED = 1.0;
      BALL_SPEED = 1.2;
      BBALL_SPEED = 1.2;
      SLOWDOWN = 0.9;
      GUMM = 20;


      BALLM = 3;
      LBALLM = 3;
      BBALLM = 8;
      APPLEM = 40;
      ROCKETM = 4;

      break;
    case 2:
      ROCKET_SPEED = 1.2;
      BALL_SPEED = 1.2;
      BBALL_SPEED = 1.2;
      SLOWDOWN = 0.8;
      GUMM = 20;


      APPLEM = 34;
      BALLM = 3;
      LBALLM = 3;
      BBALLM = 8;
      ROCKETM = 4;
      break;
    case 3:
      ROCKET_SPEED = 2.0;
      BALL_SPEED = 1.2;
      BBALL_SPEED = 1.2;
      SLOWDOWN = 0.8;
      GUMM = 20;


      BALLM = 3;
      LBALLM = 3;
      APPLEM = 24;
      BBALLM = 8;
      ROCKETM = 5;
      break;
    case 4:
      ROCKET_SPEED = 2.0;
      BALL_SPEED = 1.2;
      BBALL_SPEED = 1.2;
      SLOWDOWN = 0.8;
      GUMM = 15;


      BALLM = 3;
      LBALLM = 3;
      APPLEM = 24;
      BBALLM = 8;
      ROCKETM = 7;


    }
#ifdef NETSUPPORT
  if (!server)
#endif
    clearpoints ();
#ifdef NETSUPPORT
  if (!client)
#endif
    init_objects1 ();
  if (!server)
    effect (nos);
#ifdef NETSUPPORT
  else
    {
      if (server)
	SEffect (lastlevel, nos);
    }
#endif

}

int
allow_finder ()
{
  if (level > FINDERLEVEL || gameplan == DEATHMATCH)
    return 1;
  return 0;
}
int
create_letter ()
{
  int             tirage, chance, letter;
  int             i;
  if (gameplan == COOPERATIVE)
    {
      i = rand () % 4;
      if (i < 3)
	return (i);
      if (level > THIEFLEVEL && rand () % 400 < level - THIEFLEVEL + 40)
	return (3);
      if (level > TTOOLLEVEL && rand () % 600 < level - TTOOLLEVEL + 40)
	return (5);
      if (level > FINDERLEVEL && gtime < 0 && rand () % 700 < level - FINDERLEVEL + 40)
	return (4);
      return (0);
    }
  chance = rand () % 100;
  if (chance < 80)
    {				/* create letter : = 1 rand / 2 */
      tirage = rand () % 100;
      if (tirage < 30)
	letter = 1;
      else if (tirage < 60)
	letter = 2;
      else if (tirage < 70)
	letter = 5;
      else if (tirage < 80)
	letter = 3;
      else
	letter = 4;
      return (letter);
    }
  else
    return (0);

}
void
update_game ()
{
  int             i;
  if (gamemode == GAME)
    {
      if (ktime)
	{
	  ktime--;
	  if (ktime == 0)
	    switch (kmode)
	      {
	      case 1:
		init_objects ();
		break;
	      case 2:
		lastlevel = 0;
		level = 1, gamemode = MENU;
#ifdef NETSUPPORT
		if (server)
		  Outro ();
		else
#endif
		  outro2 ();
		break;
	      case 3:
		if (a_balls || a_bballs || a_inspectors || a_lunatics)
		  break;
		for (i = 0; i < nrockets; i++)
		  if (object[i].type == ROCKET && object[i].live)
		    object[i].score += object[i].live * 20;
		level++, init_objects ();
		break;
	      case 4:
		for (i = 0; i < nrockets; i++)
		  if (object[i].type == ROCKET)
		    object[i].score -= 100;
		nos = 1;
		init_objects ();
		nos = 0;
		break;
	      }
	}
      else
	switch (gameplan)
	  {


	  case DEATHMATCH:
	    if (nrockets == 1)
	      {
		if (!(rand () % 60))
		  creator (HOLE);
		if (a_rockets == 0)
		  {
		    ktime = 50;
		    kmode = 1;
		    object[0].score -= 100;
		  }
		if (a_lunatics == 0)
		  {
		    ktime = 50;
		    kmode = 1;
		    object[0].score += 100;
		  }
		break;
	      }
	    if (a_balls == 0)
	      secondpart = 1;
	    if (a_lunatics < nrockets && !(rand () % 150))
	      creator (LUNATIC);
	    if (secondpart)
	      {
		if (!(rand () % 100))
		  creator (BBALL);
		if (!(rand () % 60))
		  creator (HOLE);
		if (!(rand () % 400))
		  creator (BBALL);
		if (!(rand () % 400))
		  creator (INSPECTOR);
		if (!(rand () % 600))
		  creator (EHOLE);
	      }

	    if (a_rockets <= 1)
	      {
		ktime = 50;
		kmode = 1;
	      }
	    break;



	  case COOPERATIVE:
	    if (level == 99)
	      {
		if (!a_apples)
		  ktime = 50, kmode = 2;
		if (a_balls < 15)
		  if (!(rand () % 40))
		    creator (BALL);
		/*if (a_lunatics < 3)
		   if (!(rand () % 90))
		   creator (LUNATIC); */
		if (a_bballs < 3)
		  if (!(rand () % 3000))
		    creator (BBALL);
	      }
	    else
	      {
		gtime--;
		if (a_balls == 0 && a_bballs == 0 && a_inspectors == 0 && a_lunatics == 0)
		  {
		    ktime = 50, kmode = 3;
		  }
		if (a_balls < 4 * level && gtime < 0)
		  if (!(rand () % ((nrockets == 1 ? 200 : 150) + (110 - level))))
		    creator (BALL);
		if (a_lunatics < (level - LUNATICLEVEL) && a_lunatics < 3 && gtime < 0)
		  if (!(rand () % ((nrockets == 1 ? 800 : 450) + (110 - level))))
		    creator (LUNATIC);

		if (a_holes < 4 * (level - HOLELEVEL) && gtime < 0)
		  if (!(rand () % (412 + 512 / level)))
		    creator (HOLE);
		if (a_bballs < 4 * (level - BBALLLEVEL) && gtime < 0)
		  if (!(rand () % ((nrockets == 1 ? 700 : 500) + 1 * (110 - level) / 3 + 2024 / level)))
		    creator (BBALL);
		if (a_inspectors < level / (INSPECTORLEVEL) && gtime < 0)
		  if (!(rand () % (1500 + 10 * (110 - level))))
		    creator (INSPECTOR);
		if (a_eholes < level / EHOLELEVEL + 1 && gtime < 0 && level >= EHOLELEVEL)
		  if (!(rand () % (500 + 1000 / level)))
		    creator (EHOLE);
	      }
	    if (a_rockets == 0)
	      {
		ktime = 50;
		kmode = 4;
	      }
	  }
    }
  else
    {
      if (a_balls < 5 && !(rand () % (50)))
	creator (BALL);
      if (a_lunatics < 5 && !(rand () % (50)))
	creator (LUNATIC);
      if (a_bballs < 4 && !(rand () % (200)))
	creator (BBALL);
    }
}
