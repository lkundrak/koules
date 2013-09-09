/*
 * sound.h - Platform Independant Sound Support - Dec. 1994
 *
 * Copyright 1994 Sujal M. Patel (smpatel@wam.umd.edu)
 * Conditions in "copyright.h"          
 */

#if defined(SOUND) || defined(NAS_SOUND) || defined(RSOUND)

#ifdef __STDC__
void            init_sound ();	/* Init Sound System                          */
int             play_sound (int k);	/* Play a Sound                               */
void            maybe_play_sound (int k);	/* Play sound if the last 'k' sound_completed */
void            sound_completed (int k);	/* Complete a sound 'k'                       */
void            kill_sound ();	/* Terminate a sound unpredictably :)         */
#endif
#endif /* SOUND */
