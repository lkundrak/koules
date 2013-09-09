
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
*                    licence details.                      *
*----------------------------------------------------------*
*  objectsio.c reading/writing possitions of objects-for   *
*	       for network code....			   *
***********************************************************/
#ifdef NETSUPPORT
#include "koules.h"
#include "net.h"
#include "client.h"
#include "server.h"
#include <string.h>
#include <math.h>
unsigned char   b[256];
unsigned char  *buffer;
/* This code is specially optimized to produce very compressed packets
   Normal output for 1 frame=50 bytes.. game structure: 16*256=3KB 
   compression:98.88% :)) */

/*I know...bithacket and ugly bit putting routines... */
#define PUTZERO() (c<<=1,c==256?(c=1,buffer++,*buffer=0):0)
#define GETZERO() (c<<=1,c==256?(c=1,buffer++):0)
#define PUTONE() (*buffer|=c,PUTZERO())
#define PUTBIT(b) (((b)?(*buffer|=c):0),PUTZERO())
#define PUTNUM(b,n) {int co=1; for(;co<(1<<n);co<<=1) PUTBIT(b&co);}
#define GETBIT() (*buffer&c?(GETZERO(),1):(GETZERO(),0))
#define GETNUM(b,n) {int co=1;b=0; for(;co<(1<<n);co<<=1) b|=(GETBIT())?co:0;}

/*Debugging macros... */
/*#define DEBUG */
#ifdef DEBUG
#define Debug(text,num) printf(text,num),fflush(stdout);
#define Debug1(text) printf(text),fflush(stdout);
#else
#define Debug(text,num)
#define Debug1(text)
#endif
extern void     accel (int);

/*bit counter */
static int      c = 1;
unsigned char  *
write_object (unsigned char *buffer, int i)
{
  unsigned long   x;

  PUTBIT (object[i].live);
  Debug ("%i:", i);
  Debug ("Live:%i ", object[i].live);
  if (object[i].live)
    {
      PUTNUM (object[i].type, 4);
      Debug ("Type:%i ", object[i].type);
    }
  if (i < 5)
    {
      if (object[i].live)
	{
	  PUTBIT (object[i].thief);
	  PUTBIT (acceled[i]);
	}
      PUTNUM (object[i].score, 20);
      PUTNUM (object[i].live1, 3);
    }
  if (object[i].live)
    {
      if (object[i].lineto == -1)
	{
	  PUTZERO ();
	}
      else
	{
	  PUTONE ();
	  PUTNUM (object[i].lineto, 8);
	}
      Debug ("lineto:%i ", object[i].lineto);
      x = object[i].x;
      PUTNUM (x, 10);
      Debug ("x:%i ", (int) object[i].x);
      Debug ("y:%i ", (int) object[i].x);
      x = object[i].y;
      PUTNUM (x, 9);
    }
  if (object[i].live && object[i].type == ROCKET)
    {
      while(object[i].rotation<0) object[i].rotation+=2*M_PI;
      while(object[i].rotation>=2*M_PI) object[i].rotation-=2*M_PI;
      x = (object[i].rotation * 16.0 / M_PI);
      Debug ("Rotation:%i ", x);
      PUTNUM (x, 6);
    }
  if (object[i].live && object[i].type == LBALL)
    {
      Debug ("Letter:%c ", object[i].letter);
      switch (object[i].letter)
	{
	case L_ACCEL:
	  PUTNUM (1, 3);
	  break;
	case L_GUMM:
	  PUTNUM (2, 3);
	  break;
	case L_FINDER:
	  PUTNUM (3, 3);
	  break;
	case L_THIEF:
	  PUTNUM (4, 3);
	  break;
	case L_TTOOL:
	  PUTNUM (5, 3);
	  break;
	default:
	  printf ("Internal error:unknown letter type!\n");
	  PUTNUM (1, 3);
	  break;
	}
    }
  Debug1 ("\n");
  return (buffer);
}
unsigned char  *
read_object (int i, unsigned char *buffer, int nodata)
{
  unsigned long   x;
  int             a = 0;
  object[i].live = GETBIT ();
  Debug ("%i:", i);
  Debug ("Object:%i", object[i].live);
  if (object[i].live)
    {
      GETNUM (object[i].type, 4);
      Debug ("Type:%i", object[i].type);
    }
  if (i < 5)
    {
      if (object[i].live)
	{
	  object[i].thief = GETBIT ();
	  a = GETBIT ();
	}
      GETNUM (object[i].score, 20);
      GETNUM (object[i].live1, 3);
    }
  if (object[i].live)
    {
      if (GETBIT ())
	{
	  GETNUM (object[i].lineto, 8);
	}
      else
	object[i].lineto = -1;
      Debug ("Lineto:%i ", object[i].lineto);
      GETNUM (x, 10);
      object[i].x = x;
      Debug ("x:%i ", (int) object[i].x);
      GETNUM (x, 9);
      object[i].y = x;
      Debug ("y:%i ", (int) object[i].y);
    }
  if (object[i].live && object[i].type == ROCKET)
    {
      GETNUM (x, 6);
      Debug ("Rotation:%i ", x);
      object[i].rotation = (float) x *M_PI / 16.0;
    }
  if (object[i].live && object[i].type == LBALL)
    {
      GETNUM (x, 3);
      switch (x)
	{
	case 1:
	  object[i].letter = L_ACCEL;
	  break;
	case 2:
	  object[i].letter = L_GUMM;
	  break;
	case 3:
	  object[i].letter = L_THIEF;
	  break;
	case 4:
	  object[i].letter = L_FINDER;
	  break;
	case 5:
	  object[i].letter = L_TTOOL;
	  break;
	default:
	  object[i].letter = ' ';
	  printf ("Internal error:unknown letter...maybe newer server than client?\n");
	  break;
	}
      Debug ("Letter:%c", object[i].letter);
    }
  if (a)
    {
      int             n;
      for (n = 0; n < nodata; n++)
	accel (i);
    }
  Debug1 ("\n");
  return (buffer);
}
int
write_objects (unsigned char *buffer)
{
  int             i;
  unsigned char  *buffer1 = buffer;
  /*Initialize bit operation routines */
  c = 1;
  buffer[0] = 0;
  /*Optimize number of objects */
  nobjects--;
  while (object[nobjects].live == 0 && object[nobjects].time == 0)
    nobjects--;
  nobjects++;
  Debug ("%i\n", nobjects);
  /*Put number of object at the beggining of packet */
  PUTNUM (nobjects, 8);
  /*Put all objects */
  for (i = 0; i < nobjects; i++)
    buffer = write_object (buffer, i);
  if (c != 1)
    buffer++;
  return (buffer - buffer1);
}
int
read_objects (unsigned char *buffer, int nodata)
{
  int             i;
  unsigned char  *buffer1 = buffer;
  c = 1;
  GETNUM (nobjects, 8);
  Debug ("%i\n", nobjects);
  for (i = 0; i < nobjects; i++)
    buffer = read_object (i, buffer, nodata);
  for (; i < MAXOBJECT; i++)
    object[i].live = 0;
  return (buffer - buffer1);
}
#endif
