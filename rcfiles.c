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
*                     licence  details.                    *
*----------------------------------------------------------*
*  rcfiles.c saving and loading of rc files                *
***********************************************************/
/* Changes for OS/2 Warp with Dive.                        *
 *  Copyright(c)1996 by Thomas A. K. Kjaer                 *
 ***********************************************************/
/* Changes for joystick "accelerate by deflection"         *
 *  (c) 1997 by Ludvik Tesar (Ludv\'{\i}k Tesa\v{r})       *
 ************************LT*********************************/
#include <errno.h>
#include "koules.h"
#ifdef XSUPPORT
static CONST char *name = ".xkoules-controls";
#else
#ifdef OS2DIVE
static CONST char *name = "koules.cfg";
#else
static CONST char *name = ".koules.svga-controls";
#endif
#endif

#ifdef OS2DIVE
static CONST char *levelsname = "koules.lev";
#else
static CONST char *levelsname = ".koules-levels";
#endif
static char * mygetenv(char *name)
{
  static char name1[200];
  char *var=getenv(name);
  int i=strlen(var);
  if(i>199) i=199;
  memcpy(name1,var,i+1);
  name1[199]=0;
  return name1;
}

void
save_rc ()
{
  FILE           *controls, *levels;
  int             zeros[] =
  {0, 0, 0, 0, 0, 0, 0, 0};
  char            fullname[256];
#ifdef NETSUPPORT
  if (client || server)
    return;
#endif
#ifdef OS2DIVE
  sprintf (fullname, "%s", name);
#else
  sprintf (fullname, "%s/%s", mygetenv ("HOME"), name);
#endif
  if ((controls = fopen (fullname, "w")) == NULL)
    {
      printf ("could not open save file:%s\n"
	      "saving of controls skippen\n", fullname);
      perror ("error");
      return;
    }
  fwrite (keys, 1, sizeof (keys), controls);	/*save user defined keys */
#ifdef MOUSE
  fwrite (&mouseplayer, 1, sizeof (int), controls);	/*save mouseplayer */
#else
  fwrite (zeros, 1, sizeof (int), controls);
#endif
#ifdef JOYSTICK
  fwrite (joystickplayer, 1, sizeof (joystickplayer), controls);
#else
  fwrite (zeros, 2, sizeof (int), controls);
#endif
  fwrite (rotation, 1, sizeof (rotation), controls);
#ifdef JOYSTICK
  fwrite (joystickmul, 1, sizeof (joystickmul), controls);
#else
  fwrite (zeros, 2, sizeof (float), controls);
#endif
  fclose (controls);
#ifdef OS2DIVE
  sprintf (fullname, "%s", levelsname);
#else
  sprintf (fullname, "%s/%s", mygetenv ("HOME"), levelsname);
#endif
  if ((levels = fopen (fullname, "w")) == NULL)
    {
      printf ("could not open save file:%s\n"
	      "saving of controls skippen\n", fullname);
      perror ("error");
      return;
    }
  fwrite (&maxlevel, 1, sizeof (int), levels);
  fwrite (&lastlevel, 1, sizeof (int), levels);
  fwrite (&nrockets, 1, sizeof (int), levels);
  fwrite (&gameplan, 1, sizeof (int), levels);
  fwrite (&difficulty, 1, sizeof (int), levels);
  fclose (levels);
}


void
load_rc ()
{
  FILE           *controls, *levels;
  int             dummy[8];
  char            fullname[256];
#ifdef NETSUPPORT
  if (client || server)
    return;
#endif
#ifdef OS2DIVE
  sprintf (fullname, "%s", name);
#else
  sprintf (fullname, "%s/%s", mygetenv ("HOME"), name);
#endif
  if ((controls = fopen (fullname, "r")) == NULL)
    {
      printf ("could not open save file:%s\n"
	      "using default values\n", fullname);
      perror ("error");
      goto skip;
    }
  fread (keys, 1, sizeof (keys), controls);	/*save user defined keys */
#ifdef MOUSE
  if(!fread (&mouseplayer, 1, sizeof (int), controls)) mouseplayer=-1;	/*save mouseplayer */
#else
  fread (dummy, 1, sizeof (int), controls);
#endif
#ifdef JOYSTICK
  if(!fread (joystickplayer, 1, sizeof (joystickplayer), controls)) joystickplayer[0]=joystickplayer[1]=-1;
#else
  fread (dummy, 2, sizeof (int), controls);
#endif
  fread (rotation, 1, sizeof (rotation), controls);
#ifdef JOYSTICK
  fread (joystickmul, 1, sizeof (joystickmul), controls);
#else
  fread (dummy, 2, sizeof (float), controls);
#endif
  fclose (controls);
skip:;
#ifdef OS2DIVE
  sprintf (fullname, "%s", levelsname);
#else
  sprintf (fullname, "%s/%s", mygetenv ("HOME"), levelsname);
#endif
  if ((levels = fopen (fullname, "r")) == NULL)
    {
      printf ("could not open save file:%s\n"
	      "using default values\n", fullname);
      perror ("error");
      return;
    }

  fread (&maxlevel, 1, sizeof (int), levels);
  fread (&lastlevel, 1, sizeof (int), levels);
  if(maxlevel>=100||maxlevel<0) maxlevel=0;
  if(lastlevel>maxlevel||lastlevel<0) lastlevel=0;
  fread (&nrockets, 1, sizeof (int), levels);
  if(nrockets<1) nrockets=1;
  if(nrockets>4) nrockets=1;
  if (fread (&gameplan, 1, sizeof (int), levels) != sizeof (int))
                    gameplan = COOPERATIVE;
  if(gameplan<0||gameplan>1) gameplan=COOPERATIVE;
  if (fread (&difficulty, 1, sizeof (int), levels) != sizeof (int))
                    difficulty = 2;
  if(difficulty<0||difficulty>4) difficulty=COOPERATIVE;
  fclose (levels);
}
