/*
 * koules.sndsrv.sgi.c - SGI Indigo/Indigo^2/Indy Sound Driver
 *
 * Copyright (c) 1996 Rick Sayre (whorfin@pixar.com)
 *	Based on the various existing PC sound drivers.
 *
 *    Permission to use, copy, modify, and distribute this
 *   software and its documentation for any purpose and without
 *   fee is hereby granted, provided that the above copyright
 *   notice appear in all copies and that both that copyright
 *   notice and this permission notice appear in supporting
 *   documentation.  No representations are made about the
 *   suitability of this software for any purpose.  It is
 *   provided "as is" without express or implied warranty.
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <signal.h>
#include <audio.h>

#define FREQUENCY	8000
#define FRAGSIZE	510	/* 1/16 second: XXX 500 causes 6.2 crash! */

char *FILENAME[] = {
  "/start.raw",
  "/end.raw",
  "/colize.raw",
  "/destroy1.raw",
  "/destroy2.raw",
  "/creator1.raw",
  "/creator2.raw",
                   };

#define NUM_SOUNDS	(sizeof(FILENAME)/sizeof(char*))

signed char *sound_buffer[NUM_SOUNDS];

int sound_size[NUM_SOUNDS];


/* Terminate: Signal Handler */
static void 
quit()
{
    exit(0);
}



static void 
init(int argc, char **argv)
{
    int             i;
    char            s[1024];

    if (argc != 3) {
	printf("This program is only executed by koules\n");
	exit(1);
    }
    for (i = 0; i < NUM_SOUNDS; i++) {
	s[0] = 0;
	strcat(s, argv[1]);
	if (s[(int) strlen(s) - 1] == '/')
	    FILENAME[i]++;
	strcat(s, FILENAME[i]);
	FILENAME[i] = malloc((int) strlen(s));
	strcpy(FILENAME[i], s);
	sound_buffer[i] = NULL;
	sound_size[i] = 0;
    }

    signal(SIGTERM, quit);	/* Setup Terminate Signal Handler */
}



static ALport 
setup_dsp(char *dspdev)
{
    ALport          writePort;
    ALconfig        writeConfig;

    static long     audioParams[2] = {
	AL_OUTPUT_RATE, FREQUENCY
    };

    writeConfig = ALnewconfig();
    ALsetqueuesize(writeConfig, FRAGSIZE);
    ALsetchannels(writeConfig, AL_MONO);
    ALsetparams(AL_DEFAULT_DEVICE, audioParams, 2);
    ALsetwidth(writeConfig, AL_SAMPLE_8);

    if (!(writePort = ALopenport("koules.sndsrv", "w", writeConfig))) {
	fprintf(stderr, "koules.sndsrv.sgi:  Couldn't open audio port\n");
	return(NULL);
    }

    return writePort;
}

/*
   This just keeps the pipe from breaking...
   Eventually I'll look at the koules signal handlers and
   just trap this.
*/
static void
do_nothing(void)
{
    fprintf(stderr, "koules.sndsrv: doing nothing, something is broken\n");
    while (1)
	sleep(5);
}

static int 
read_sound(int k)
{
    int             i, fd, size;

#ifdef DEBUG
    fprintf(stderr, "loading sound %d, %s\n", k, FILENAME[k]);
#endif

    fd = open(FILENAME[k], O_RDONLY);
    if (fd <= 0) {
	fprintf(stderr, "koules.sndsrv: The sound %s could not be opened\n", FILENAME[k]);
	sound_size[k] = -1;
	return (0);
    }
    size = lseek(fd, 0, SEEK_END);
    sound_size[k] = (size / FRAGSIZE) + 1;	/* size in fragments */
    sound_buffer[k] = malloc(sound_size[k] * FRAGSIZE);
    if (sound_buffer[k] == NULL) {
	fprintf(stderr, "koules.sndsrv: couldn't malloc memory for sound\n");
	sound_size[k] = -1;
	close(fd);
	return (0);
    }
    lseek(fd, 0, SEEK_SET);
    read(fd, sound_buffer[k], size);
    close(fd);

    /* data is unsigned; convert to signed */
    for (i = 0; i < size; i++)
	sound_buffer[k][i] ^= 0x80;
    bzero(sound_buffer[k] + size, sound_size[k] * FRAGSIZE - size);

#ifdef DEBUG
    fprintf(stderr, "sound has been loaded, %d bytes\n", size);
#endif
    return (1);
}


static void 
do_everything(ALport writePort)
{
    signed char     k;
    int             i, j;
    int             terminate = -1;	/* Which Sound to Terminate */
    int             playing[16];/* Sound numbers that we are playing */
    int             position[16];	/* Current position in each sound
					 * file */
    int             playnum = 0;/* Number of sounds currently being played */
    signed char   final[512];	/* Final Mixing Buffer */
    int             premix[512];
    signed char    *sample;

    for (;;) {
	terminate = -1;

	/* Try to open a new sound if we get an integer on the 'in' pipe */
	i = read(STDIN_FILENO, &k, sizeof(k));
	if (i == 0) {		/* EOF on pipe means parent has closed its end*/
#ifdef DEBUG
	    fprintf(stderr,"koules.sndsrv: shutting down\n");
#endif
	    kill(getpid(), SIGTERM);
	}
	if (i != -1) {		/* there was something in the pipe */
#ifdef DEBUG
	     fprintf(stderr,"Just read a %d from pipe\n",(int)k);
#endif
	    /* Negative means terminate the FIRST sound in the buffer */
	    if (k < 0) {
#ifdef DEBUG
		fprintf(stderr,"terminating sound\n");
#endif
		terminate = 0;
	    } else {
		if (sound_size[(int) k] == 0)
		    read_sound(k);
		if (sound_size[(int) k] > 0 && playnum < 16) {
		    position[playnum] = 0;
		    playing[playnum++] = k;
#ifdef DEBUG
		    fprintf(stderr,"sound %d added to play queue\n",playnum-1);
#endif
		}
	    }
	}

	/* terminate a sound if necessary */
	for (i = 0; i < playnum; i++) {
	    if ((position[i] == sound_size[playing[i]]) || (terminate == i)) {
#ifdef DEBUG
		 fprintf(stderr,"finished playing sound %d\n",i);
		 fprintf(stderr,"is was at position %d\n",position[i]);
#endif
		bcopy(playing + i + 1, playing + i, (playnum - i) * sizeof(int));
		bcopy(position + i + 1, position + i, (playnum - i) * sizeof(int));
		playnum--;
		i--;
	    }
	}

	if (playnum) {
	    /* Mix each sound into the final buffer */
	    bzero(premix, sizeof(premix));
	    for (i = 0; i < playnum; i++) {
		sample = sound_buffer[playing[i]] + position[i] * FRAGSIZE;
		for (j = 0; j < FRAGSIZE; j++) {
		    premix[j] += *(sample + j);
		}
		position[i]++;
	    }
	    /* clamp premix */
	    for (i = 0; i < FRAGSIZE; i++)
		final[i] = (premix[i] > 127) ? 127 : (premix[i] < -128 ? -128 : premix[i]);
	} else {
	    /*
	     * We have no sounds to play Just fill the buffer with silence
	     * and maybe play it
	     */
	    bzero(final, sizeof(final));
	}
	ALwritesamps(writePort, final, FRAGSIZE);

    }
}



void 
main(int argc, char **argv)
{
    ALport          writePort;

    fcntl(STDIN_FILENO, F_SETFL, O_NONBLOCK);
    init(argc, argv);

    if (!(writePort = setup_dsp(argv[2])))
	do_nothing();

    do_everything(writePort);
}
