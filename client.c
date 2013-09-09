
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
*  client.c network client routines                        *
***********************************************************/
#include <stdio.h>
#include <sys/time.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include "sock.h"
#include "net.h"
#include "client.h"
#include "koules.h"
#ifdef NETSUPPORT
#define error printf
#ifdef SOUND
extern int      ssound;
#endif
extern void     process_keys ();
extern int      fadedout;
extern int      nplayers;
extern int      explosion (int x, int y, int type, int letter, int n);
extern int      creators_points (int radius, int x, int y, int color);
extern void     setnplayers (int n);
extern int      read_objects (unsigned char *buffer, int n);
extern void     effect (int nos);
extern void     cmenu2 (unsigned char *message, int size);
extern void     client_loop2 (int draw);
char            servername[256];
static int      socket_c;
static int      sock;
static int      nodata;
static int      quited = 0;
static int      port;
static int      falied;
static int      allframes, dispframes, realdisplayed;
static int      rframe;
static long     rpos;
static long     rcount = 1;
static long     rrcount = 0;
static long     timeout = 1;
static int      nodisplayco = 0;
static char     hostname[256];
#define BUFFERSIZE 65536
static unsigned char buffer[BUFFERSIZE];
struct c_table
  {
    void            (*func) (unsigned char *message, int size);
  };
static unsigned char obuffer[BUFFERSIZE];
static unsigned char ibuffer[BUFFERSIZE];
static unsigned char rbuffer[BUFFERSIZE];
static int      rposition;
static int      obufferpos;
#define WAITTIME 5
void
                csendbegining (unsigned char *message, int size);

void
csetrrcount (long scount)
{
  int             size, pos = 0, count;
  if (scount <= rrcount)
    return;			/*Old repeated message */
  do
    {
      GETLONG ((rbuffer + pos), count);
      GETSHORT ((rbuffer + pos) + 4, size);
      pos += size;
    }
  while (count < scount);
  memmove (rbuffer, rbuffer + pos, rposition - pos);
  rposition -= pos;
  rrcount = scount;
}

void
cflushreliable ()
{
  if (rposition)
    {
      PUTHEAD (SRELIABLE) memcpy (buffer + HEADSIZE, rbuffer, rposition);
      csendbegining (buffer, rposition + HEADSIZE);
    }
}

void
csendbuffer ()
{
  int             result;
  if (timeout)
    {
      timeout--;
      if (!timeout)
	{
	  cflushreliable ();
	  timeout = WAITTIME;
	}
    }
  if (!obufferpos)
    return;
  PUTLONG ((obuffer), rpos);
  falied = 0;
  GetSocketError (sock);
  /*if (!(rand()%2)) *//*Emulate internet :)))))) */
  if ((result = DgramSend (sock, servername, port,
			   (char *) obuffer, obufferpos + 4)) <= 0)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK || result >= 0)
	{
	  GetSocketError (sock);
	  falied = 1;
	  if (timeout == WAITTIME)
	    timeout = 1;
	}
      else
	{
	  perror ("Can't send message to server\n");
	  SocketClose (socket_c);
	  SocketClose (sock);
	  exit(-1);
	}
    }
  obufferpos = 0;
}
void
csendbegining (unsigned char *message, int size)	/*send at more reliable beggining of packet */
{
  if (obufferpos + size + 6 > BUFFERSIZE)
    csendbuffer ();

  memmove (obuffer + size + 6, obuffer + 4, obufferpos);
  PUTSHORT ((obuffer + 4), size + 2);
  memcpy (obuffer + 6, message, size);
  obufferpos += size + 2;
}
void
csendreliable (unsigned char *message, int size)
{
  if (size == 0)
    return;
  if (rposition + size + 6 > BUFFERSIZE)
    {
      printf ("Connection to server broken too long! Reliable buffer owerfllow\n"
	      "Game aborted\n");
      exit (-1);
    }
  PUTLONG ((rbuffer + rposition), rcount);
  rcount++;
  PUTSHORT ((rbuffer + rposition + 4), (size + 6));
  memcpy (rbuffer + rposition + 6, message, size);
  rposition += size + 6;
  timeout = 1;
}

void
csend (unsigned char *message, int size)
{
  if (obufferpos + size + 2 + 4 > BUFFERSIZE)
    csendbuffer ();
  PUTSHORT ((obuffer + obufferpos + 4), size + 2);
  memcpy (obuffer + obufferpos + 6, message, size);
  obufferpos += size + 2;
}
void
register_players ()
{
  PUTHEAD (SREGISTER);
  PUTCHAR ((buffer + 1), nplayers);
  csendreliable (buffer, HEADSIZE + 1);
  gamemode = WAIT;
}
void
start_game ()
{
  PUTHEAD (SSTART);
  csendreliable (buffer, HEADSIZE);
  gamemode = WAIT;
#ifdef SOUND
  sound = ssound;
#endif
}
void
CQuit (char *text)
{
  static int      n;
  if (quited)
    return;
  quited = 1;
  PUTHEAD (SQUIT);
  strcpy ((char *) (buffer + HEADSIZE), text);
  printf ("Terminating game.\nReason:%s\n", text);
  for (n = 0; n < 5; n++)
    {				/*repeat quiting messages to make sure that
				   all clients received it */
      csend (buffer, strlen (text) + HEADSIZE + 1);
      csendbuffer ();
    }
  SocketClose (sock);
  quited = 1;
  /*exit(0); */
  uninitialize ();
  exit (0);
}

void
GetPos (void)
{
#if 0
  int             x = /*(dispframes + realdisplayed + 1) / 2 */ dispframes / 4;
#else
  int             x = realdisplayed + 5;
#endif
  PUTHEAD (SPOS);
  PUTCHAR (buffer + HEADSIZE, x);
  csendreliable (buffer, HEADSIZE + 1);
}
void
SendKeys (int player, int c)
{
  PUTHEAD (SKEY);
  PUTCHAR (buffer + HEADSIZE, c);
  PUTCHAR (buffer + HEADSIZE + 1, player);
  csendreliable (buffer, HEADSIZE + 2);
}
void
SendRotation (int player, int c)
{
  PUTHEAD (SROT);
  PUTCHAR (buffer + HEADSIZE, c);
  PUTCHAR (buffer + HEADSIZE + 1, player);
  csendreliable (buffer, HEADSIZE + 2);
}
void
SendMouse (int player, int x, int y, int buttons)
{
  PUTHEAD (SMOUSE);
  PUTSHORT (buffer + HEADSIZE, x);
  PUTSHORT (buffer + HEADSIZE + 2, y);
  PUTCHAR (buffer + HEADSIZE + 4, buttons);
  PUTCHAR (buffer + HEADSIZE + 5, player);
  csendreliable (buffer, HEADSIZE + 6);
}
void
SendJoystick (int player, int x, int y, int buttons)
{
  PUTHEAD (SJOYSTICK);
  PUTSHORT (buffer + HEADSIZE, x);
  PUTSHORT (buffer + HEADSIZE + 2, y);
  PUTCHAR (buffer + HEADSIZE + 4, buttons);
  PUTCHAR (buffer + HEADSIZE + 5, player);
  csendreliable (buffer, HEADSIZE + 6);
}
void
SendControls ()
{
  static struct control c[5];
  int             i;
  for (i = 0; i < 5; i++)
    {
      if (control[i] && memcmp ((char *) &controls[i], (char *) &c[i], sizeof (controls[i])))
	{
	  memcpy ((char *) &c[i], (char *) &controls[i], sizeof (controls[i]));
	  switch (c[i].type)
	    {
	    case C_KEYBOARD:
	      SendKeys (i, c[i].mask);
	      break;
	    case C_RKEYBOARD:
	      SendRotation (i, c[i].mask);
	      break;
	    case C_MOUSE:
	      SendMouse (i, c[i].mx, c[i].my, c[i].mask);
	      break;
	    case C_JOYSTICK1:
	      SendMouse (i, c[i].jx, c[i].jy, c[i].mask);
	      break;
	    }
	}
    }
}
void
Ready (void)
{
  PUTHEAD (SREADY);
  csendreliable (buffer, HEADSIZE);
}
static void
print (unsigned char *message, int size)
{
  printf ("Server's message:%s\n", (char *) message);
}
static void
quit (unsigned char *message, int size)
{
  printf ("Uninitializing:%s\n", (char *) message);
  SocketClose (sock);
  quited = 1;
  exit (0);
}
static void
cexplosion (unsigned char *message, int size)
{
  int             x, y, type, letter, n;
  GETSHORT (message, x);
  GETSHORT ((message + 2), y);
  GETCHAR ((message + 4), type);
  GETCHAR ((message + 5), letter);
  GETCHAR ((message + 6), n);
  explosion (x, y, type, letter, n);
}
static void
coutro (unsigned char *message, int size)
{
  outro2 ();
  CQuit ("End of game\nBye");
}
static void
ccreatord_points (unsigned char *message, int size)
{
  int             x, y, radius, color;
  GETSHORT (message, x);
  GETSHORT ((message + 2), y);
  GETSHORT ((message + 4), radius);
  GETCHAR ((message + 6), color);
  creators_points (radius, x, y, color);
}
static void
csound (unsigned char *message, int size)
{
  int             s;
  GETCHAR (message, s);
  Effect (s, next);
}

static void
cplayers (unsigned char *message, int size)
{
  int             s;
  GETCHAR (message, s);
  setnplayers (s);
}
static void
creg (unsigned char *message, int size)
{
  int             s;
  for (s = 0; s < MAXROCKETS; s++)
    if (message[s])
      control[s] = 1;
    else
      control[s] = 0;
}
static void
cpos (unsigned char *message, int size)
{
  read_objects ((unsigned char *) message, nodata);
  rframe = 1;
  nodata = 0;
}
static void
clevel (unsigned char *message, int size)
{
  int             nos;
  GETCHAR (message, lastlevel);
  GETCHAR (message + 1, nos);
  GETCHAR (message + 2, gameplan);
  gamemode = GAME;
#ifdef SOUND
  sound = ssound;
#endif
  gameplan_init ();
  effect (nos);
  nodisplayco = 10;
  Ready ();
}
static void
cgame (unsigned char *message, int size)
{
  gamemode = GAME;
  Ready ();
}
void            creliable (unsigned char *message, int size);
static struct c_table ctable[] =
{
  {print},
  {quit},
  {cpos},
  {cexplosion},
  {ccreatord_points},
  {csound},
  {cplayers},
  {creg},
  {cmenu2},
  {clevel},
  {cgame},
  {creliable},
  {coutro}
};
void
creliable (unsigned char *message, int size)
{
  int             pos = 0;
  while (pos < size)
    {
      long            rpos1;
      int             size;
      GETLONG ((message + pos), rpos1);
      if (rpos1 > rpos + 1)
	{
	  printf ("Fatal error in transfering reliable messages!\n"
		  "Game may become VERY unstable..please contact author \n");
	  return;
	}
      GETSHORT ((message + pos + 4), size);
      if (rpos1 <= rpos)
	{
	  pos += size;
	  continue;
	}
      rpos = rpos1;
      ctable[message[pos + 6]].func ((message + pos + 7), size - 7);
      pos += size;
    }
}
void
process_message (void)
{
  int             bytes, pos = 4;
  long            scount;
  if (!SocketReadable (sock))
    return;
  GetSocketError (sock);
  if ((bytes = DgramReceiveAny (sock, (char *) ibuffer, BUFFERSIZE)) <= 0)
    {
      if (errno == EAGAIN || errno == EWOULDBLOCK || bytes >= 0)
	{
	  GetSocketError (sock);
	  return;
	}
      CQuit ("Can't receive server's message\n");
      SocketClose (sock);
    }
  GETLONG (ibuffer, scount);
  csetrrcount (scount);
  while (pos + 2 < bytes)
    {
      int             size;
      GETSHORT (ibuffer + pos, size);
      if (pos + size <= bytes)
	{
	  ctable[ibuffer[pos + 2]].func ((ibuffer + pos + 3), size - 3);
	}
      pos += size;
    }

}
#define VERSION 0
int
init_client (void)
{
  long            bytes, pport;
  int             gx, gy;
  int             retries = 0;
/*Open server socket */
  printf ("Opening server socket\n");
  if ((socket_c = CreateDgramSocket (0)) == -1)
    {
      error ("Could not create connection socket");
      SocketClose (socket_c);
      exit (-1);
    }
  strcpy(hostname,GetSockAddr(socket_c));
/*Create connection message */
  strcpy ((char *) buffer, "Koules");
  PUTCHAR (buffer + 7, VERSION);
  PUTSHORT (buffer + 8, GAMEWIDTH);
  PUTSHORT (buffer + 10, GAMEHEIGHT);
  PUTLONG (buffer + 12, (long) getpid ());
  printf ("Asking server at %s port %i\n", servername, initport);
#if 0
  if (DgramConnect (socket_c, servername, initport) == -1)
    {
      perror ("Connection error");
      error ("Can't connect to server %s on port %d", servername, port);
      close (socket_c);
      return -1;
    }
#endif
again:
  GetSocketError (socket_c);
  errno = 0;
  do
    {
      if (retries)
	sleep (1);
      retries++;
      if (retries > 100)
	printf (" Connection timed out\n"), exit (-1);
      if (errno)
	{
	  perror ("Can't receive reply");
	  exit (-1);
	}
      if (DgramSend (socket_c, servername, initport,
		     (char *) buffer, INITPACKETSIZE) == -1)
	{
	  SocketClose (socket_c);
	  perror ("Can't send message to server");
	  exit (-1);
	}
      if (errno)
	{
	  perror ("Can't send message to server");
	  /*exit (-1); */
	  GetSocketError (socket_c);
	  errno = 0;
	}
      SetTimeout (1, 10 * 1000);
    }
  while (!(SocketReadable (socket_c)));
  bytes = DgramReceiveAny (socket_c, (char *) buffer, BUFFERSIZE);
  if (bytes != REPLYSIZE)
    goto again;
  SocketClose (socket_c);
  GETLONG (buffer, port);
  if (port == 0)
    {
      printf ("Server refused me! (too many players or incompatible screen size)\n"
	      "Try -W server's option is you are using 320x200 clients..\n");
      exit (-1);
    }
  GETSHORT ((buffer + 4), gx);
  GETSHORT ((buffer + 6), gy);

  printf ("YYYYAAAAAAA Server replied.\n"
	  "Opening port %i\n", port);
  if ((sock = CreateDgramSocket (0)) == -1)
    {
      error ("Can't create datagram socket");
      return -1;
    }
  SetTimeout (1, 10 * 1000);

  /*if (DgramConnect (sock, servername, port) == -1)
     {
     error ("Can't connect to server %s on port %d", servername, port);
     close (sock);
     return -1;
     } */
  if (SetSocketNonBlocking (sock, 1) == -1)
    {
      error ("Can't make socket non-blocking");
      return -1;
    }
  printf ("Sending initialization message\n");
  pport = GetPortNum (sock);
  if (gx / DIV > GAMEWIDTH || gy / DIV > GAMEHEIGHT)
    {
      printf ("Server's gamepool too large\n"
	      "Try -W on server's command line\n");

      PUTLONG (buffer, 0);
      if (DgramSend (sock, servername, port,
		     (char *) buffer, 4) == -1)
	close (sock);
      exit (-1);
    }
  else
    GAMEWIDTH = gx, GAMEHEIGHT = gy;
  PUTHEAD (SINIT);
  PUTLONG (buffer + HEADSIZE, pport);
  if (SetSocketReceiveBufferSize (sock, NETBUFFER) == -1)
    {
      error ("Can't set socket buffer size");
      return -1;
    }
  if (SetSocketSendBufferSize (sock, NETBUFFER) == -1)
    {
      error ("Can't set socket buffer size");
      return -1;
    }
  GetSocketError (sock);
  csendreliable (buffer, 4 + HEADSIZE);
  csendbuffer ();
  csendbuffer ();
  csendbuffer ();
  printf ("Starting game...\n");
  return (0);
}
void
client_loop (void)
{
  long            VfTime = 0;
  long            VendSleep = 0;
  struct timeval  VlastClk;
  struct timeval  VnewClk;
  int             wait = 0;
#if 0
  int             drawed = 0;
  int             display = 1;
#endif
  client = 1;

  load_rc ();
  init_menu ();

  Ready ();
#ifdef XSUPPORT
  nopause = 1;
#endif
  gamemode = MENU;
  gettimeofday (&VlastClk, NULL);
  gettimeofday (&VnewClk, NULL);
  VendSleep = VlastClk.tv_usec;
  VfTime = 1000000 / 25;
  dispframes = 25 * 4;
  realdisplayed = 25;
  GetPos ();
  dispframes = 0;
  realdisplayed = 0;
  while (1)
    {
      if (nodisplayco)
	nodisplayco--;
      SetTimeout (0, 0);
      nodata++;
      while (SocketReadable (sock))
	process_message ();
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      allframes++;
      if (fadedout)
	rframe = 1;
      process_keys ();
      if ((wait > 0 || tbreak))
	{			/*display=0; */
	  csendbuffer ();
	  realdisplayed++;
	  client_loop2 (rframe && (!nodisplayco));
	  rframe = 0;
	}
      else
	{
	  csendbuffer ();
	  client_loop2 (0);
	}
      if (allframes == 25 || tbreak)
	{
	  /*printf ("Displayed frames:%i really:%i\n", dispframes / 4, realdisplayed); */
	  if (tbreak)
	    realdisplayed = 25;
	  GetPos ();
	  allframes = 0;
	  dispframes = 0;
	  realdisplayed = 0;
	}
      SendControls ();
#ifndef HAVEUSLEEP
      do
	{			/*my emulation of usleep isn't reiable on HP-UX machines
				   when sockets are comming :( */
#endif
	  gettimeofday (&VnewClk, NULL);
	  if (VnewClk.tv_usec < VendSleep)
	    VendSleep -= 1000000;
	  wait = (VfTime - VnewClk.tv_usec + VendSleep);
	  if (tbreak)
	    wait = VfTime;
	  if (wait > 0)
	    usleep (wait);
#ifndef HAVEUSLEEP
	}
      while (wait > 10 && !tbreak);
#endif
      VendSleep = VnewClk.tv_usec + wait;
      gettimeofday (&VlastClk, NULL);
      if (tbreak)
	VendSleep = VlastClk.tv_usec;
      tbreak = 0;
    }
}
#endif
