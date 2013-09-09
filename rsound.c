/* NCD Audio format - original code by Dave Lemke <lemke@verbosa.ncd.com> */
/* Modified by paul kendall for X Galaga. */

/*
 *  Include file dependencies:
 */
#ifdef RSOUND
#include <stdio.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stddef.h>
#include <ctype.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/fcntl.h>

#include "rplay.h"
#include <X11/Xlib.h>
#include "koules.h"

char           *unixSoundPath = SOUNDDIR;
#define playSounds 1

/*
 *  Internal variable declarations:
 */

static char    *FILENAME[] =
{
  "/start.au",
  "/end.au",
  "/colize.au",
  "/destroy1.au",
  "/destroy2.au",
  "/creator1.au",
  "/creator2.au"
};
#define NUM_SOUNDS      (sizeof(FILENAME)/sizeof(char*))


#define	MAX_SOUNDS	8
static int      fd;

static int      audio_on = False;

void
init_sound ()
{
  char           *displayname = DisplayString (dp);
  char            host[256], *p, s[256];
  int             i;

  if (audio_on)
    return;
  strcpy (host, displayname);

  if ((p = strrchr (host, (int) ':')) != NULL)
    *p = 0;

  if (!*host)
    strcat (host, "localhost");

  printf ("Directing sound to: %s\n", host);

  if ((fd = rplay_open (host)) < 0)
    {
      rplay_perror (host);
      return;
    }
  audio_on = True;
  for (i = 0; i < NUM_SOUNDS; i++)
    {
      s[0] = 0;
      strcat (s, SOUNDDIR);
      if (s[(int) strlen (s) - 1] == '/')
	FILENAME[i]++;
      strcat (s, FILENAME[i]);
      FILENAME[i] = malloc ((int) strlen (s) + 1);
      strcpy (FILENAME[i], s);
    }


  return;

}

static void    *p[7];

void
playSoundFile (char *filename, int volume, void **private)
{
  RPLAY         **p = (RPLAY **) private;


  if (!*p)
    {
      printf ("loading sound %s\n", filename);
      *p = rplay_create (RPLAY_PLAY);
      rplay_set (*p, RPLAY_INSERT, 0, RPLAY_SOUND, strdup (filename), NULL);
    }

  rplay_set (*p, RPLAY_CHANGE, 0, RPLAY_VOLUME, volume, NULL);
  rplay (fd, *p);

}


int
play_sound (k)
     int             k;
{
  char            c;

  c = k;
  if (audio_on)
    {
      playSoundFile (FILENAME[k], 50, &p[k]);
    }
  return (0);
}


void
maybe_play_sound (k)
     int             k;
{
}

void
sound_completed (k)
     int             k;
{
}

void
kill_sound ()
{
}
#endif
