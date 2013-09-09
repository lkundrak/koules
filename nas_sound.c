
/* NCD Audio format - original code by Dave Lemke <lemke@verbosa.ncd.com> */
/* Modified by paul kendall for X Galaga. */

/*
 *  Include file dependencies:
 */
#ifdef NAS_SOUND
#include <stdio.h>
#ifdef __STDC__
#include <stdlib.h>
#endif
#include <unistd.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>
#include <signal.h>
#include <sys/stat.h>
#include <audio/audiolib.h>
#include <audio/soundlib.h>
#include <X11/Xlib.h>
#include "koules.h"

char           *unixSoundPath = SOUNDDIR;
#define playSounds 1

/*
 *  Internal variable declarations:
 */

#define	MAX_SOUNDS	8

AuServer       *aud = NULL;
AuDeviceID      device;
static int      audio_on = False;
static int      num_sounds = 0;
static char     errorString[255];

static struct
  {
    char           *name;
    char           *filename;
    void           *private;
  }
sound_table[MAX_SOUNDS];

typedef struct
  {
    int             playing;
    AuBucketID      bucket;
  }
audioRec       , *audioPtr;

void
init_sound ()
{
  int             i;
  char           *displayname = DisplayString (dp);
  if (unixSoundPath[0] == '?')
    {
      return;
    };
  if (audio_on)
    return;

  /* try and connect to the NCD audio server */
  if (!(aud = AuOpenServer (displayname, 0, NULL, 0, NULL, NULL)))
    {
      return;
    }

  /* Look for an audio device that we can use */
  for (i = 0; i < AuServerNumDevices (aud); i++)
    {
      if ((AuDeviceKind (AuServerDevice (aud, i)) ==
	   AuComponentKindPhysicalOutput) &&
	  AuDeviceNumTracks (AuServerDevice (aud, i)) == 1)
	{
	  device = AuDeviceIdentifier (AuServerDevice (aud, i));
	  break;
	}
    }

  /* Well we didn't get a device - all busy? */
  if (!device)
    {
      AuCloseServer (aud);
      return;
    }

#if defined(SOUNDLIB_VERSION) && SOUNDLIB_VERSION >= 2
  AuSoundRestartHardwarePauses = AuFalse;
#endif

  /* Success - we have an audio device */
  audio_on = True;

  return;
}

static void
doneCB (aud, handler, event, info)
     AuServer       *aud;
     AuEventHandlerRec *handler;
     AuEvent        *event;
     audioPtr        info;
{
  info->playing = False;
}

void
audioDevicePlay (filename, volume, private)
     char           *filename;
     int             volume;
     void          **private;
{
  audioPtr       *info = (audioPtr *) private;

  if (!*info)
    {
      if (!(*info = (audioPtr) malloc (sizeof (audioRec))))
	{
	  return;
	}

      (*info)->playing = 0;
      (*info)->bucket = AuSoundCreateBucketFromFile (aud, filename,
					      AuAccessAllMasks, NULL, NULL);
    }

/*    if ((*info)->bucket && (!(*info)->playing)) */
  if ((*info)->bucket)
    {
      (*info)->playing = 1;
      AuSoundPlayFromBucket (aud, (*info)->bucket, device,
			     AuFixedPointFromFraction (volume, 100),
	  (void (*)) doneCB, (AuPointer) * info, 1, NULL, NULL, NULL, NULL);

      /* Flush sound */
      AuFlush (aud);
    }
}

void
check_sound ()
{
  if (aud)
    AuHandleEvents (aud);
}

void
playSoundFile (filename, volume)
     char           *filename;
     int             volume;
{
  int             i;
  char            fbuf[1024];
  char           *str;

  /* Loop through the sound table looking for sound */
  for (i = 0; i < num_sounds; i++)
    {
      if (!strcmp (sound_table[i].name, filename))
	{
	  /* Yeah - already in sound table */
	  break;
	}
    }

  /* Ok - not found so add it to the sound table */
  if (i == num_sounds)
    {
      /* new one - so add it to the table */
      sound_table[num_sounds].name = strdup (filename);

      /* Use the environment variable if it exists */
      if ((str = getenv ("XGAL_SOUND_DIR")) != NULL)
	sprintf (fbuf, "%s/%s", str, filename);
      else
	sprintf (fbuf, "%s/%s", unixSoundPath, filename);

      sound_table[num_sounds].filename = strdup (fbuf);
      num_sounds++;
    }

  audioDevicePlay (sound_table[i].filename, volume, &sound_table[i].private);
}

char           *FILENAME[] =
{
  "/start.au",
  "/end.au",
  "/colize.au",
  "/destroy1.au",
  "/destroy2.au",
  "/creator1.au",
  "/creator2.au"

};

int
play_sound (k)
     int             k;
{
  char            c;

  c = k;
  if (playSounds && aud)
    {
      playSoundFile (FILENAME[k], 50);
    }
  return 0;
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
