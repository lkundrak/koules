/* Sound module for Koules/2
 *
 * This module loads the RAW sample datas and plays it
 */  

#define INCL_OS2MM
#define INCL_DOSPROCESS
#define INCL_DOSSEMAPHORES
#include <os2.h>
#include <os2me.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>

#ifdef DEBUG
#include <stdio.h>
#endif /*  */

/* prototypes */ 
void            read_sound (int);


#define NUMBER_OF_COMMANDS 2

/* RAW sample parameters */ 
#define SAMPLESPERSEC 8000
#define BITSPERSAMPLE    8
#define CHANNELS         1

/* The samples to load */ 
char           *FILENAME[] = 
{
 "start.raw", 
 "end.raw", 
 "colize.raw", 
 "destroy1.raw", 
 "destroy2.raw", 
 "creator1.raw", 
 "creator2.raw" 
};


#define NUM_SOUNDS	(sizeof(FILENAME)/sizeof(char*))


signed char    *sound_buffer[NUM_SOUNDS];

int             sound_size[NUM_SOUNDS];

#define          fragsize 512

typedef struct pls 
  {
    
    ULONG ulCommand;
    
    ULONG ulOperandOne;
    
    ULONG ulOperandTwo;
    
    ULONG ulOperandThree;
    
  }
PLAY_LIST_STRUCTURE;


/* This is the memory playlist we'll be using */ 
PLAY_LIST_STRUCTURE PlayList[NUMBER_OF_COMMANDS] = 
{
  
    DATA_OPERATION, 0, 0, 0, /* play command      */ 
    EXIT_OPERATION, 0, 0, 0 /* terminate command */  
};



BOOL soundPlaying = FALSE;

HEV hevSoundEvent;

MCI_OPEN_PARMS mciOpenParameters;	/* Open structure.        */

MCI_WAVE_SET_PARMS mwspWaveFormParameters;	/* Waveform parameters.   */

USHORT usSoundDeviceID;		/* device ID              */

TID tidSoundThread;		/* sound-thread ID        */


int             fd[2];		/* for unnamed pipe */

int             soundfd;

int             wavtoPlay;


extern int      sound;


/* experimental real-time mixing code */ 
#ifdef 0
void
__play_sound (void) 
{
  
    char            k;
  
    int             i, j;
  
    int             terminate = -1;
  
    int             playing[16];	/* sound numbers that we are playing */
  
    int             position[16];	/* Current position in each file     */
  
    int             playnum = 0;	/* number of sounds currently being played */
  
    int             finalBuf = 0;
  
    signed char     final[10 * fragsize];	/* 10 buffers */
  
    int             premix[fragsize];
  
    
    char           *sample;
  
    ULONG mciRC;
  
    
#ifdef DEBUG
    printf ("starting SoundThread\n");
  
#endif /*  */
    fcntl (soundfd, F_SETFL, O_NDELAY);
  
    for (;;)
    
    {
      
	i = read (soundfd, &k, sizeof (k));
      
#ifdef DEBUG
	printf ("i=%d, k=%d, playnum=%d\n", i, (int) k, (int) playnum);
      
#endif /*  */
	if (i == 0)		/* EOF */
	
	  _endthread ();
      
	
	if (i != -1)		/* there was something in the pipe */
	
	{
	  
#ifdef DEBUG
	    printf ("Just read a %d from pipe\n", (int) k);
	  
#endif /*  */
	    if (k < 0)
	    
	    {
	      
		terminate = 0;
	      
	    }
	  
	    else
	    
	    {
	      
		if (sound_size[(int) k] > 0 && playnum < 16)
		
		{
		  
		    position[playnum] = 0;
		  
		    playing[playnum++] = k;
		  
#ifdef SOUND
		    printf ("Adding sound %d to queue, total %d\n", (int) k, playnum);
		  
#endif /*  */
		}
	      
	    }
	  
	}
      
	
      /* terminate a sound if necessary */ 
	for (i = 0; i < playnum; i++)
	
	{
	  
	    if ((position[i] == sound_size[playing[i]]) || (terminate == i))
	    
	    {
	      
#ifdef SOUND
		printf ("removing %d\n", (int) i);
	      
#endif /*  */
		memmove (playing + i, 
			 playing + i + 1, 
			 (playnum - i) * sizeof (int));
	      
		memmove (position + i, 
			 position + i + 1, 
			 (playnum - i) * sizeof (int));
	      
		playnum--;
	      
		i--;
	      
	    };
	  
	};
      
	
	if (playnum)		/* play sound */
	
	{
	  
	    for (i = 0; i < playnum; i++)
	    
	    {
	      
#ifdef SOUND
		printf ("sample %d at position %d\n", playing[i], position[i]);
	      
#endif /*  */
		memset (premix, 0, sizeof (premix));
	      
		
		sample = sound_buffer[playing[i]] + position[i] * fragsize;
	      
		for (j = 0; j < fragsize; j++)
		
		{
		  
		    premix[j] += *(sample + j);
		  
		};
	      
		position[i]++;
	      
	    };
	  
	    for (i = 0; i < fragsize; i++)
	    {
	      
		final[i + finalBuf * fragsize] = (premix[i] > 255) ? 255 : 
		(premix[i] < -256 ? 0 : (premix[i] >> 1) + 128);
	      
	      /*            final[i+finalBuf*fragsize] ^=0x80; */ 
	    }
	  
	}
      
	else
	
	{
	  
	  /* 
	     We have no sounds to play
	     Just fill the buffer with silence and maybe play it 
	   */ 
	    memset (final + finalBuf * fragsize, 0, fragsize);
	  
	}
      
	
	
      /* setup wave file in playlist */ 
	PlayList[0].ulOperandOne = (ULONG) (final + finalBuf * fragsize);
      
	PlayList[0].ulOperandTwo = fragsize;
      
	
	mciRC = mciSendCommand (usSoundDeviceID, 
				MCI_STOP, 
				MCI_WAIT, 
				(PVOID) & mciOpenParameters, 
				0);
      
	
	if (mciRC != 0)
	{
	  
	    sound = FALSE;
	  
#ifdef DEBUG
	    printf ("Error MCI_STOP device\n");
	  
#endif /*  */
	}
      
	
      /* rewind sound */ 
	mciRC = mciSendCommand (usSoundDeviceID, 
				MCI_SEEK, 
				MCI_TO_START, 
				(PVOID) & mciOpenParameters, 
				0);
      
	
      /* play sound */ 
	mciRC = mciSendCommand (usSoundDeviceID, 
				MCI_PLAY, 
				MCI_WAIT, 
				(PVOID) & mciOpenParameters, 
				0);
      
	
	if (mciRC != 0)
	{
	  
	    sound = FALSE;
	  
#ifdef DEBUG
	    printf ("Error MCI_PLAY device\n");
	  
#endif /*  */
	}
      
	
	finalBuf++;
      
	if (finalBuf == 10)
	finalBuf = 0;
      
	
    }
  
    
    _endthread ();
  
}


void
play_sound (int k) 
{
  
    char            c;
  
    
    c = k;
  
    if (sound)
    
      write (fd[1], &c, sizeof (c));
  
}


#endif /*  */


int
play_sound (int k) 
{
  
    ULONG mciRC;
  
    
    if (TRUE)
    {
      
	wavtoPlay = k;
      
	
	mciRC = mciSendCommand (usSoundDeviceID, 
				MCI_STOP, 
				MCI_WAIT, 
				(PVOID) & mciOpenParameters, 
				0);
      
	if (mciRC != 0)
	{
	  
	    sound = FALSE;
	  
#ifdef DEBUG
	    printf ("Error MCI_STOP device\n");
	  
#endif /*  */
	}
      
	
      /* rewind sound */ 
	mciRC = mciSendCommand (usSoundDeviceID, 
				MCI_SEEK, 
				MCI_TO_START, 
				(PVOID) & mciOpenParameters, 
				0);
      
	
	if (mciRC != 0)
	{
	  
	    sound = FALSE;
	  
#ifdef DEBUG
	    printf ("Error MCI_SEEK device\n");
	  
#endif /*  */
	}
      
	
	
	DosPostEventSem (hevSoundEvent);
      
    }
  
    
    return 0;
  
}


/* Tries to play sound k, if anything fails, sound will be disabled. */ 
void
__play_sound (void) 
{
  
    int             k;
  
    ULONG mciRC;
  
    ULONG ulCount;
  
    
    while (TRUE)
    {
      
	DosWaitEventSem (hevSoundEvent, SEM_INDEFINITE_WAIT);
      
	soundPlaying = TRUE;
      
	k = wavtoPlay;
      
	
	PlayList[0].ulOperandOne = (ULONG) sound_buffer[k];
      
	PlayList[0].ulOperandTwo = (sound_size[k] + 1) * fragsize;
      
	
      /* play sound */ 
	mciRC = mciSendCommand (usSoundDeviceID, 
				MCI_PLAY, 
				MCI_WAIT, 
				(PVOID) & mciOpenParameters, 
				0);
      
	
	if (mciRC != 0)
	{
	  
	    sound = FALSE;
	  
#ifdef DEBUG
	    printf ("Error MCI_PLAY device\n");
	  
#endif /*  */
	    
	}
      
	
	soundPlaying = FALSE;
      
	DosResetEventSem (hevSoundEvent, &ulCount);
      
    }
  
    
    _endthread ();
  
}



/* opens the device and reads the samples */ 
void
init_sound (void) 
{
  
    int             i;
  
    
    ULONG mciRC;
  
    ULONG ulOpenFlags = (MCI_WAIT | MCI_OPEN_PLAYLIST | 
			 MCI_OPEN_TYPE_ID);
  
    
  /* Open the correct waveform device for the waves with MCI_OPEN */ 
    mciOpenParameters.pszDeviceType = (PSZ) 
    MAKEULONG (MCI_DEVTYPE_WAVEFORM_AUDIO, 1);
  
    
  /* The address of the buffer containing the waveform file. */ 
    mciOpenParameters.pszElementName = (PSZ) & PlayList[0];
  
    
    mciOpenParameters.hwndCallback = (HWND) NULL;
  
    mciOpenParameters.pszAlias = (CHAR) NULL;
  
    
  /* Open the waveform file in the playlist mode. */ 
    mciRC = mciSendCommand (0, 
			    MCI_OPEN, 
			    ulOpenFlags, 
			    (PVOID) & mciOpenParameters, 
			    0);
  
    
    if (mciRC != 0)
    {
      
	sound = FALSE;
      
#ifdef DEBUG
	printf ("Error opening device\n");
      
#endif /*  */
	return;
      
    }
  
    
  /* save device ID */ 
    usSoundDeviceID = mciOpenParameters.usDeviceID;
  
    
  /* Fill the structure with zeros. */ 
    memset (&mwspWaveFormParameters, 0, sizeof (mwspWaveFormParameters));
  
    
  /* copy samps/sec */ 
    mwspWaveFormParameters.ulSamplesPerSec = SAMPLESPERSEC;
  
    mwspWaveFormParameters.usBitsPerSample = BITSPERSAMPLE;
  
    mwspWaveFormParameters.usChannels = CHANNELS;
  
    mwspWaveFormParameters.ulAudio = MCI_SET_AUDIO_ALL;
  
    
    mciRC = mciSendCommand (usSoundDeviceID, 
			    MCI_SET, 
			    (MCI_WAIT | 
			     MCI_WAVE_SET_SAMPLESPERSEC | 
			     MCI_WAVE_SET_BITSPERSAMPLE | 
			     MCI_WAVE_SET_CHANNELS), 
			    (PVOID) & mwspWaveFormParameters, 
			    0);
  
    if (mciRC != 0)
    {
      
	sound = FALSE;
      
#ifdef DEBUG
	printf ("Error setting device\n");
      
#endif /*  */
	return;
      
    }
  
    
#ifdef DEBUG
    printf ("%d sounds to be loaded\n", (int) NUM_SOUNDS);
  
#endif /*  */
    
  /* read the samples */ 
    for (i = 0; i < NUM_SOUNDS; i++)
    read_sound (i);
  
    
  /* open unnamed pipe */ 
    if ((pipe (fd)) != 0)
    {
      
#ifdef DEBUG
	printf ("error creating pipe\n");
      
#endif /*  */
    }
  
    
  /* make the read pipe, non-blocking */ 
    fcntl (fd[0], F_SETFL, O_NDELAY);
  
    fcntl (fd[1], F_SETFL, O_NDELAY);
  
    
    soundfd = fd[0];
  
    
    DosCreateEventSem (NULL, &hevSoundEvent, 0, FALSE);
  
  /* start soundthread */ 
    tidSoundThread = _beginthread ((void *) __play_sound, NULL, 16384L, NULL);
  
    DosSetPriority (PRTYS_THREAD, PRTYC_TIMECRITICAL, 0, tidSoundThread);
  
}



void
read_sound (int k) 
{
  
    int             fd, size, i;
  
    char            filename[20];
  
    
#ifdef DEBUG
    printf ("Opening no.%d %s\n", k, FILENAME[k]);
  
#endif /*  */
    sprintf (filename, "sounds/%s", FILENAME[k]);
  
    fd = open (filename, O_RDONLY);
  
    if (fd <= 0)
    
    {
      
#ifdef DEBUG
	fprintf (stderr, "koules.sndsrv: The sound %s number %i could not be opened\n", FILENAME[k], k);
      
#endif /*  */
	sound_size[k] = -1;
      
	return;
      
    };
  
    size = lseek (fd, 0, SEEK_END);
  
    sound_size[k] = (size / fragsize) + 1;
  
#ifdef DEBUG
    printf ("size = %d\n", (int) sound_size[k]);
  
#endif /*  */
    sound_buffer[k] = malloc ((sound_size[k] + 1) * fragsize);
  
    lseek (fd, 0, SEEK_SET);
  
    read (fd, sound_buffer[k], size);
  
    close (fd);
  
    
/*
   for (i = 0; i < size; i++)
   sound_buffer[k][i] ^= 0x80;
 */ 
    
  /* fill rest of buffer with 0's */ 
    memset (sound_buffer[k] + size, 0, sound_size[k] * fragsize - size);
  
}



void
close_device (void) 
{
  
    MCI_GENERIC_PARMS mciGenericParms;
  
    
  /* close devide */ 
    mciSendCommand (mciOpenParameters.usDeviceID, 
		    MCI_CLOSE, 
		    MCI_WAIT, 
		    (PVOID) & mciGenericParms, 
		    (ULONG) NULL);
  
  /* close the pipe */ 
    close (fd[0]);
  
    close (fd[1]);
  
}



#ifdef DEBUG
VOID play_wave (USHORT usSoundFileID) 
{
  
    ULONG mciRC;
  
    
  /* setup wave file in playlist */ 
    PlayList[0].ulOperandOne = (ULONG) sound_buffer[usSoundFileID];
  
    PlayList[0].ulOperandTwo = (sound_size[usSoundFileID] + 1) * fragsize;
  
    
    mciRC = mciSendCommand (usSoundDeviceID, 
			    MCI_STOP, 
			    MCI_WAIT, 
			    (PVOID) & mciOpenParameters, 
			    0);
  
    
    if (mciRC != 0)
    {
      
	sound = FALSE;
      
#ifdef DEBUG
	printf ("Error MCI_STOP device\n");
      
#endif /*  */
    }
  
    
  /* rewind sound */ 
    mciRC = mciSendCommand (usSoundDeviceID, 
			    MCI_SEEK, 
			    MCI_TO_START, 
			    (PVOID) & mciOpenParameters, 
			    0);
  
    
    if (mciRC != 0)
    {
      
	sound = FALSE;
      
#ifdef DEBUG
	printf ("Error MCI_SEEK device\n");
      
#endif /*  */
    }
  
    
  /* play sound */ 
    mciRC = mciSendCommand (usSoundDeviceID, 
			    MCI_PLAY, 
			    MCI_WAIT, 
			    (PVOID) & mciOpenParameters, 
			    0);
  
    
    if (mciRC != 0)
    {
      
	sound = FALSE;
      
#ifdef DEBUG
	printf ("Error MCI_PLAY device\n");
      
#endif /*  */
    }
  
}

#endif /*  */


#ifdef DEBUG
int             sound = TRUE;


int
main () 
{
  
    init_sound ();
  
    
/*      play_wave(1); */ 
    play_sound (0);
  
    
    play_sound (1);
  
    play_sound (2);
  
    play_sound (3);
  
    play_sound (4);
  
    play_sound (5);
  
    while (1);
  
    
    close_device ();
  
    
    return 0;
  
}

#endif /*  */
