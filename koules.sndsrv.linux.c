/*
 * koules.sndsrv.c - VoxWare(tm) Compatible Sound - Apr. 1995
 *                 PC Speaker  Compatible Sound 
 *                 This server is Linux Specific.
 *
 * Copyright 1994-1995 Sujal M. Patel (smpatel@wam.umd.edu)
 * Conditions in "copyright.h"
 *
 * Modified for koules and bugfixed by Jan Hubicka
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/soundcard.h>
#include "linux_pcsp.h"		/* /usr/include/linux/pcsp.h      */
#include <sys/time.h>
#include <signal.h>
#include <string.h>



char           *FILENAME[] =
{
  "/start.raw",
  "/end.raw",
  "/colize.raw",
  "/destroy1.raw",
  "/destroy2.raw",
  "/creator1.raw",
  "/creator2.raw"

};

#define NUM_SOUNDS	(sizeof(FILENAME)/sizeof(char*))

signed char    *sound_buffer[NUM_SOUNDS];
int             sound_size[NUM_SOUNDS];
int             fragsize;


/* Terminate: Signal Handler */
void
quit ()
{
  exit (0);
}



void
init (int argc, char **argv)
{
  int             i;
  char            s[1024];

  if (argc != 3)
    {
      printf ("This program is only executed by koules\n");
      exit (1);
    }

  for (i = 0; i < NUM_SOUNDS; i++)
    {
      s[0] = 0;
      if (argv[1][(int) strlen (argv[1]) - 1] == '/')
	FILENAME[i]++;
      snprintf(s, sizeof(s), "%s%s", argv[1], FILENAME[i]);
      FILENAME[i] = malloc ((int) strlen (s) + 1);
      strcpy (FILENAME[i], s);
      sound_buffer[i] = NULL;
      sound_size[i] = 0;
    }

  signal (SIGTERM, quit);	/* Setup Terminate Signal Handler */
}


/*
   Setup DSP: Opens /dev/dsp or /dev/pcdsp
   Sets fragment size on VoxWare
   Sets speed to 8000hz
   Should set mono mode
   Error checking                
 */
int
setup_dsp (char *dspdev, int *is_pcsp)
{
  int             dsp, frag, value;

  dsp = open (dspdev, O_WRONLY);
  if (dsp < 0)
    {
      fprintf (stderr, "koules.sndsrv: Couldn't open DSP >%s<\n", dspdev);
      perror("error");
      return -1;
    }
  *is_pcsp = 0;
  fragsize = 0;

  frag = 0x00020007;		/* try 512 bytes, for 1/16 second frag size */
  ioctl(dsp, SNDCTL_DSP_SAMPLESIZE, &value);
  if(ioctl (dsp, SNDCTL_DSP_SETFRAGMENT, &frag)) 
  {
      fprintf (stderr, "koules.sndsrv: Couldn't set DSP fragment. Sounds will be ugly and delayed. Use USS lite driver!\n");
  }
  value = 8010;
  if (ioctl (dsp, SNDCTL_DSP_SPEED, &value))
    {
      fprintf (stderr, "koules.sndsrv: Couldn't set DSP rate!\n");
    };
  value = 0;
  ioctl (dsp, SNDCTL_DSP_STEREO, &value);
  value = 1;
  ioctl (dsp, SNDCTL_DSP_SYNC, &value);
  ioctl (dsp, SNDCTL_DSP_GETBLKSIZE, &fragsize);
  value=8;
  ioctl(dsp, SNDCTL_DSP_SAMPLESIZE, &value);

  if (!fragsize)
    {
      /* Don't Assume just because you can't set the fragment, use proper IOCTL */
      fprintf (stderr, "koules.sndsrv: Couldn't set Fragment Size.\nAssuming PC Speaker!\n");
      fragsize = 128;
      *is_pcsp = 1;
    }

  return dsp;
}

/*
   This just keeps the pipe from breaking...
   Eventually I'll look at the koules signal handlers and
   just trap this.
 */
int
do_nothing (void)
{
  fprintf (stderr, "koules.sndsrv: doing nothing, something is broken\n");
  while (1)
    sleep (5);
}

int
read_sound (int k)
{
  int             i, fd, size;

  fd = open (FILENAME[k], O_RDONLY);
  if (fd <= 0)
    {
      fprintf (stderr, "koules.sndsrv: The sound %s number %i could not be opened\n", FILENAME[k], k);
      sound_size[k] = -1;
      return (0);
    }
  size = lseek (fd, 0, SEEK_END);
  sound_size[k] = (size / fragsize) + 1;	/*size in fragments */
  sound_buffer[k] = malloc ((sound_size[k] + 1) * fragsize);
  if (sound_buffer[k] == NULL)
    {
      fprintf (stderr, "koules.sndsrv: couldn't malloc memory for sound\n");
      sound_size[k] = -1;
      close (fd);
      return (0);
    };
  lseek (fd, 0, SEEK_SET);
  read (fd, sound_buffer[k], size);
  close (fd);
  for (i = 0; i < size; i++)
    sound_buffer[k][i] ^= 0x80;
  memset (sound_buffer[k] + size, 0, sound_size[k] * fragsize - size);

  /*fprintf(stderr,"sound has been loaded, %d bytes\n",size); *//*DEBUG */
  return (1);
}


void
do_everything (int dsp, int is_pcsp)
{
  signed char     k;
  int             i, j;
  int             terminate = -1;	/* Which Sound to Terminate                              */
  int             playing[16];	/* Sound numbers that we are playing                     */
  int             position[16];	/* Current position in each sound file */
  int             playnum = 0;	/* Number of sounds currently being played               */
  unsigned char   final[512];	/* Final Mixing Buffer                                   */
  int             premix[512];
  char           *sample;
  audio_buf_info  info;

  for (;;)
    {
      terminate = -1;
      /* Try to open a new sound if we get an integer on the 'in' pipe */
      i = read (STDIN_FILENO, &k, sizeof (k));
      if (i == 0)
	{			/* EOF on pipe means parent has closed its end */
	  fprintf(stderr,"koules.sndsrv: shutting down\n"); 
	  kill (getpid (), SIGTERM);
	  exit(-1);
	};
      if (i != -1)
	{			/* there was something in the pipe */
	  /*fprintf(stderr,"Just read a %d from pipe\n",(int)k); *//*DEBUG */
	  /* Negative means terminate the FIRST sound in the buffer */
	  if (k < 0)
	    {
	      /*fprintf(stderr,"terminating sound\n"); *//*DEBUG */
	      terminate = 0;
	    }
	  else
	    {
	      if (sound_size[(int) k] == 0)
		read_sound (k);
	      if (sound_size[(int) k] > 0 && playnum < 16)
		{
		  position[playnum] = 0;
		  playing[playnum++] = k;
		  /*fprintf(stderr,"sound %d added to play queue\n",playnum-1); *//*DEBUG */
		};
	    };
	};

      /* terminate a sound if necessary */
      for (i = 0; i < playnum; i++)
	{
	  if ((position[i] == sound_size[playing[i]]) || (terminate == i))
	    {
	      /*fprintf(stderr,"finished playing sound %d\n",i); *//*DEBUG */
	      /*fprintf(stderr,"is was at position %d\n",position[i]); *//*DEBUG */
	      memmove (playing + i, playing + i + 1, (playnum - i) * sizeof (int));
	      memmove (position + i, position + i + 1, (playnum - i) * sizeof (int));
	      playnum--;
	      i--;
	    };
	};

      if (playnum)
	{
	  /* Mix each sound into the final buffer */
	  memset (premix, 0, sizeof (premix));
	  for (i = 0; i < playnum; i++)
	    {
	      sample = sound_buffer[playing[i]] + position[i] * fragsize;
	      for (j = 0; j < fragsize; j++)
		{
		  premix[j] += *(sample + j);
		};
	      position[i]++;
	    };
	  for (i = 0; i < fragsize; i++)
	    final[i] = (premix[i] > 255) ? 255 : (premix[i] < -256 ? 0 : (premix[i] >> 1) + 128);
	}
      else
	{
	  /* 
	     We have no sounds to play
	     Just fill the buffer with silence and maybe play it 
	   */
	  memset (final, 128, sizeof (final));
	};
      write (dsp, final, fragsize);
      if(!ioctl(dsp,SNDCTL_DSP_GETOSPACE,&info)) { /*this is code 
			drivers that does not allow to set number of
			fragments to 2. (ultrasound project?)*/
	while((int)info.fragments<((int)info.fragstotal-2))  {
	  usleep(1000000*info.fragsize/8010);
          ioctl(dsp,SNDCTL_DSP_GETOSPACE,&info);
	}
      } 
      /*
         The sound server is in a tight loop, EXCEPT for this
         write which blocks.  Any optimizations in the above
         code would really be helpful.  Right now the server
         takes up to 7% cpu on a 486DX/50.
       */
    }
}



int
main (argc, argv)
     int             argc;
     char          **argv;
{
  int             dsp, is_pcsp;

  fcntl (STDIN_FILENO, F_SETFL, O_NONBLOCK);
  init (argc, argv);
  dsp = setup_dsp (argv[2], &is_pcsp);
  if (dsp<0) {
	  fprintf(stderr,"Sound server: exit\n");
	  exit(0);
  }
printf("2\n");

  do_everything (dsp, is_pcsp);
printf("3\n");
  return 1;
}
