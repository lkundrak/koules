/*
 * sound.c - Platform Independant Sound Support - Apr. 1995
 *
 * Copyright 1994-1995 Sujal M. Patel (smpatel@wam.umd.edu)
 * Conditions in "copyright.h"          
 *
 * Modified for koules and bugfixed by Jan Hubicka
 */
#ifdef SOUND
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/stat.h>

char           *unixSoundPath = SOUNDDIR;
char           *unixSoundDev = SOUNDDEV;
#define playSounds 1

static int      soundfd;
static char     audioOK = 1;
static char     sound_flags[20];	/* Sound Flag for sound 1-19 */
int             child;
void
test_sound ()
{
  if (audioOK)
    if (kill (child, 0))
      {
	audioOK = 0;
	close (soundfd);
      };
}

void
init_sound ()
{
  int             i, fd[2];
  char           *argv[4];
  char            filename[512];

  signal (SIGCHLD, SIG_IGN);
  signal (SIGPIPE, SIG_IGN);

  if (unixSoundPath[0] == '?')
    {
      audioOK = 0;
      return;
    };

  /*  Create a pipe, set the write end to close when we exec the sound server,
     and set both (is the write end necessary?) ends to non-blocking   */
  pipe (fd);
  soundfd = fd[1];

  if (!(child = fork ()))
    {
      close (fd[1]);
      dup2 (fd[0], STDIN_FILENO);
      close (fd[0]);
      sprintf (filename, SOUNDSERVER);
      argv[0] = filename;
      argv[1] = unixSoundPath;
      argv[2] = unixSoundDev;
      argv[3] = NULL;
      execvp (filename, argv);
      fprintf (stderr, "Couldn't Execute Sound Server %s!\n", filename);
      exit (0);
    };
  close (fd[0]);

  for (i = 0; i < 19; i++)
    sound_flags[i] = 0;
}

int
play_sound (k)
     int             k;
{
  char            c;
  test_sound ();
  c = k;
  if ((playSounds) && (audioOK))
    write (soundfd, &c, sizeof (c));
  return 0;
}



void
maybe_play_sound (k)
     int             k;
{
  char            c;

  if (sound_flags[k] & 1)
    return;

  sound_flags[k] |= 1;
  test_sound ();
  c = (unsigned char) (k);
  if ((playSounds) && (audioOK))
    write (soundfd, &c, sizeof (c));
}



void
sound_completed (k)
     int             k;
{
  sound_flags[k] &= ~1;
}



void
kill_sound ()
{
  signed char            c;

  c = -1;
  test_sound ();
  if ((playSounds) && (audioOK))
    write (soundfd, &c, sizeof (c));
}
#endif
