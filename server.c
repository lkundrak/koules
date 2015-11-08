
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
*  server.c network server routines                        *
***********************************************************/
#ifdef AVOID_HPUX_CC_BUG
#pragma OPT_LEVEL 1
#endif
/* Server part of network code..It is bit ugly but - my first network
   program at all. It use nice and powerfull socklib from xpilot game.
   Thanks to authors. It use unreiable UDP datagram sockets because
   stream ones are too slow for as fast game. It needs to transfer packet
   to every client by every frame= 25*number of clients packets per second.
   Game packet has folowin format:

   1 byte header + data. Header is number of function(event).

   Game packets are buffered and sent in bigger ones every freame.
   They have following format:

   2 byte size, Game packet, 2 byte size, Game packet .....4byte number
   of last reiable packet.

   I also use special code for reiable packets. (some of events are critical
   so they must be transfered-as registering of players,changing of game mode,
   starting new level etc...I use special buffer for them..They are buffered
   and sent to clients every 5 frames util server will not receive piggyback
   from client(at the end of normal packet) Reiable packets are transfered as
   normal game packets using special CREIABLE ans SREIABLE Game function.

   Logining part of server is not done yet...it hope that all packets
   are transfered reiably..maybe you will need more tryes to login
   client...I will make it better later.

   Code is ugly because koules weren't designed as network game :(
   But it seems to work very well...I need to improve some thinks
   before official release..

   I now trying to minimalize network transfer...it transfers about 1KB
   to every client per second now..it may work over 9600 serial line but slip
   adding packet header-40 bytes so it transfer more than 2KB and needs
   about 15000 baud line :(((
   I dont know how to make support for slow line and do not slow down
   unreiable ones yet more...I think there are two 100% different situation
   serial line-very slow but reiable
   internet - quite fast(sometimes) but unreiable (sometimes)
 */
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include "sock.h"
#include "koules.h"
#include "server.h"
#include "net.h"
#ifdef NETSUPPORT
#define  max_connections 5
#define STATISTICS
#ifdef STATISTICS
#define MAXPACKET 12
static long     frames = 0, packets = 0, size = 0;
static long     ipackets = 0, isize = 0;
int             serverstartlevel = 0;
int             servergameplan = COOPERATIVE;
static long     sizes[MAXPACKET];
static long     isizes[MAXPACKET];
char            acceled[5];
#endif
struct control  controls[MAXROCKETS];
static unsigned char Clientmap[5];
static int      Socket;
/*static int      tbreak; */
#define BUFFERSIZE 65536
static unsigned char buffer[BUFFERSIZE];
int             initport = DEFAULTINITPORT;
static int      connected;
#define WAITTIME 5		/*time for piggybacking in frames(1/25 sec) */
unsigned char   ibuffer[BUFFERSIZE];
static struct conn
  {
    int             socket;	/*socket to client */
    int             port;	/*port of client */
    int             version;	/*client's version...not used yet */
    int             connected;
    long            magic;	/*magic number */
    char            hostname[1024];
    unsigned char   obuffer[BUFFERSIZE];	/*output buffer for unreliable messages */
    unsigned char   rbuffer[BUFFERSIZE];	/*reliable messages */
    unsigned char   initmessage[REPLYSIZE];	/*initialization message */
    int             counter;
    int             itimeout;	/*timeout for resending initialization packet */
    int             timeout;
    int             failed;
    int             sent;
    int             opos;	/*possition in output buffer */
    int             rpos;	/*possition in reliable */
    int             dispframes;
    int             sentframes;
    int             allframes;
    long            rcount;	/*counter of reiable messages */
    long            rrcount;	/*counter of reiable messages received from client */
    long            ircount;	/*counter of reiable messages sent by client */
    int             ready;	/*is client ready to receive messages? */
    /*int wpos; *//*want server receive possition informations? */
  }
conn[6]        , *connp;
struct s_table
  {
    void            (*func) (int client, unsigned char *message, int size);
  };


#define A(c) (*s=c,s++)
static char     str[30];
static char    *s;
void
edisplay (int e, int l, int t)
{				/*Magic routine */
  A ("(<|]<["[l ? (e >> 4) % 7 : 0]), A ((l ? (t ? "8b8d8|" : ":;:|:8") : "||||||")[(e >> 2) % 6]),
    A (e > 40 && l ? '=' : "-^"[l ? (e >> 3) % 2 : 0]), A ((l ? (!t ? ")>D)]|(<[|" : ">OQ><(C(P|") : "|"
				       "||||||||")[(e >> 1) % 10]), A (' ');
}
#undef A

void
extesions ()
{
  static int      e[5];
  int             i;
  if (gamemode != GAME)
    return;
  s = str;
  for (i = 0; i < nrockets; i++)
    {
      if (object[i].live)
	e[i]++;
      else
	e[i] -= 2;
      if (e[i] < 0)
	e[i] = 0;
      edisplay (e[i], object[i].live, object[i].thief);
    }
  if (nrockets)
    {
      *s = 0;
      printf ("%s\r", str);
      fflush (stdout);
    }
}
void
statistics ()
{
  int             i;
  if (!packets || !ipackets || !frames)
    return;
  printf (" S E R V E R   S T A T I S T I C S\n"
	  "==================================\n"
	  "output:\n"
	  "total packets    :%i\n"
	  "total frames     :%i\n"
	  "total bytes      :%i\n"
	  "bytes per packet :%.2f\n"
	  "bytes per frame  :%.2f\n"
	  "bytes per second :%.2f\n",
	  (int) packets, (int) frames, (int) size, (float) size / packets, (float) size / frames,
	  (float) size * 25 / frames);
  for (i = 0; i < MAXPACKET; i++)
    printf ("%3i:%3i", (int) i, (int) sizes[i]);
  printf ("\ninput:\n"
	  "total packets    :%i\n"
	  "total frames     :%i\n"
	  "total bytes      :%i\n"
	  "bytes per packet :%.2f\n"
	  "bytes per frame  :%.2f\n"
	  "bytes per second :%.2f\n",
	  (int) ipackets, (int) frames, (int) isize, (float) isize / ipackets, (float) isize / frames,
	  (float) isize * 25 / frames);

  for (i = 0; i < MAXPACKET; i++)
    printf ("%3i:%3i", (int) i, (int) isizes[i]);
  printf ("\nTotal bytes transfered:%i\n"
	  "Total per second:%.2f\n"
	  "Bytes per second over slip line(40 bytes heads):%.2f=%i bps\n",
	  (int) (isize + size), (float) (isize + size) * 25 / (frames),
	  (float) (isize + size + 40 * (ipackets + packets)) * 25 / frames,
      (int) ((isize + size + 40 * (ipackets + packets)) * 25 / frames * 8));

  printf ("\n"
	  "=================================\n");
}
void
sinit (int client, unsigned char *message, int size)
{
  GETLONG (message, conn[client].port);
#if 0
  if (SetSocketNonBlocking (conn[client].socket, 0) == -1)
    {
      return;
    }
  SetTimeout (0, 10 * 1000);
  if (DgramConnect (conn[client].socket, conn[client].hostname, conn[client].port) == -1)
    {
      Quit ("Can't connect to client");
      return;
    }
  GetSocketError (conn[client].socket);
  SetTimeout (0, 0);
  if (SetSocketNonBlocking (conn[client].socket, 1) == -1)
    {
      return;
    }
  conn[client].connected = 1;
#endif
  if (!conn[client].port)
    {
      Quit ("client refused connection...strange\n");
      return;
    }
  conn[client].itimeout = 0;
  printf ("Client at %s connected and initialized!\n", conn[client].hostname);
}
void
sprint (int client, unsigned char *message, int size)
{
  printf ("Client's #%i message:%s\n", client, message);
}
void
squit (int client, unsigned char *message, int size)
{
  char            str[400];
  sprintf (str, "Server received quit message from client %i:", client);
  strcat (str, (char *) message);
  Quit (str);
}
extern int      write_objects (unsigned char *buffer);
void
spos (int client, unsigned char *message, int s)
{
  if (conn[client].ready)
    {
      int             x;
      GETCHAR (message, x);
      /*conn[client].wpos = 1; */
      conn[client].dispframes = x * 2;	/*Take some time for transfer next information */
      conn[client].allframes = 25 * 2;
      conn[client].sentframes = 0;
    }
}
void
sjoystick (int client, unsigned char *message, int s)
{
  int             p, x;
  GETCHAR (message + 5, p);
  controls[p].type = C_JOYSTICK1;
  GETSHORT (message, x);
  controls[p].jx = x;
  GETSHORT (message + 2, x);
  controls[p].jy = x;
  GETCHAR (message + 4, controls[p].mask);
}
void
smouse (int client, unsigned char *message, int s)
{
  int             p;
  GETCHAR (message + 5, p);
  controls[p].type = C_MOUSE;
  GETSHORT (message, controls[p].mx);
  GETSHORT (message + 2, controls[p].my);
  GETCHAR (message + 4, controls[p].mask);
}
void
skeys (int client, unsigned char *message, int s)
{
  int             p;
  GETCHAR (message + 1, p);
  controls[p].type = C_KEYBOARD;
  GETCHAR (message, controls[p].mask);
}
void
srot (int client, unsigned char *message, int s)
{
  int             p;
  GETCHAR (message + 1, p);
  controls[p].type = C_RKEYBOARD;
  GETCHAR (message, controls[p].mask);
}
void
sready (int client, unsigned char *message, int s)
{
  conn[client].ready = 1;
  /*conn[client].wpos = 1; */
  conn[client].dispframes = 0;	/*Take some time for transfer next information */
  conn[client].allframes = 0;
  conn[client].sentframes = 0;
  /*statistics (); */
  if (gamemode == PREGAME)
    {
      int             i;
      for (i = 0; i < connected; i++)
	if (!conn[client].ready)
	  return;
      gamemode = GAME;
    }
}
void
sreg (int client, unsigned char *message, int s)
{
  int             n, i;
  static int      registered = 0;
  registered++;
  GETCHAR (message, n)
    PUTHEAD (CREG);
  printf ("Client %i trying to register %i players\n", client, n);
  /*statistics (); */
  for (i = 0; i < MAXROCKETS; i++)
    buffer[i + HEADSIZE] = 0;
  for (i = nrockets; i < nrockets + n && i < MAXROCKETS; i++)
    {
      buffer[i + HEADSIZE] = 1;
      Clientmap[i] = client;
    }
  nrockets += n;
  if (nrockets > MAXROCKETS)
    nrockets = MAXROCKETS;
  ssendreliable (client, buffer, HEADSIZE + MAXROCKETS);
  PUTHEAD (CPLAYERS);
  PUTCHAR (buffer + 1, nrockets)
    ssendallreliable (buffer, HEADSIZE + 1);
  if (registered == connected)
    {
      printf ("All clients sucesfully registered. Entering configure mode\n");
      close (Socket), Socket = -1;
      PUTHEAD (CMENU2);
      ssendallreliable (buffer, HEADSIZE);
    }
}
void
SEffect (int level, int nos)
{
  int             i;
  printf ("Entering level %i\n", level);
  /*statistics (); */
  PUTHEAD (CLEVEL);
  PUTCHAR (buffer + 1, lastlevel)
    PUTCHAR (buffer + 2, nos)
    PUTCHAR (buffer + 3, gameplan)
    for (i = 0; i < connected; i++)
    conn[client].ready = 0;
  ssendallreliable (buffer, HEADSIZE + 3);
  gamemode = PREGAME;
}
void
sstart (int client, unsigned char *message, int s)
{
  static int      registered = 0;
  registered++;
  lastlevel = serverstartlevel;
  printf ("Client %i registered to game\n", client);
  /*statistics (); */
  /*PUTHEAD (CLEVEL);
     PUTCHAR (buffer+1,lastlevel)
     PUTCHAR (buffer+2,0)
     ready[client]=0;
     ssend ( client,buffer, HEADSIZE+2); */
  if (registered == connected)
    {
      printf ("All clients sucesfully registered. Starting game\n");
      PUTHEAD (CGAME);
      ssendallreliable (buffer, HEADSIZE);
      gamemode = GAME;
      gameplan = servergameplan;
      gameplan_init ();
      init_objects ();
      gamemode = PREGAME;
    }
}
void            sreliable (int i, unsigned char *message, int size);
struct s_table  stable[] =
{
  {sprint},
  {squit},
  {spos},
  {sreg},
  {sstart},
  {sready},
  {sjoystick},
  {smouse},
  {srot},
  {skeys},
  {sreliable},
  {sinit},
};
#define error(text) fprintf(stderr,text);
void
sreliable (int i, unsigned char *message, int size)
{
  int             pos = 0;
  while (pos < size)
    {
      long            rpos1;
      int             size;
      GETLONG ((message + pos), rpos1);
      if (rpos1 > conn[i].ircount + 1)
	{
	  printf ("Fatal error in transfering reliable messages!\n"
		  "Game may become VERY unstable..please contact author \n");
	  return;
	}
      GETSHORT ((message + pos + 4), size);
      if (rpos1 <= conn[i].ircount)
	{
	  pos += size;
	  continue;
	}
      conn[i].ircount = rpos1;
      stable[message[pos + 6]].func (i, (message + pos + 7), size - 7);
      pos += size;
    }
}

static void
closeSockets ()
{
  int             n;
  statistics ();
  SocketClose (Socket);
  for (n = 0; n < connected; n++)
    {
      SocketClose (conn[n].socket);
    }
}

void
sflushreliable (int i)
{
  if (conn[i].rpos)
    {
      PUTHEAD (CRELIABLE) memcpy (buffer + HEADSIZE, conn[i].rbuffer, conn[i].rpos);
      ssendbegining (i, buffer, conn[i].rpos + HEADSIZE);
    }
}
void
ssendbuffer (int client)
{
  int             result;
  GetSocketError (conn[client].socket);
  if (conn[client].itimeout)
    {
      conn[client].itimeout--;
      if (!conn[client].itimeout)
	{
	  if ((result = DgramSend (Socket, conn[client].hostname, conn[client].port,
				   (char *) conn[client].initmessage, sizeof (conn[client].initmessage))) <= 0)
	    {
	      if (errno == EWOULDBLOCK || errno == EAGAIN || result >= 0)
		{
		  GetSocketError (Socket);
		  conn[client].itimeout = 1;
		}
	      else
		{
		  perror ("Connection error:");
		  Quit("Can not send message to client");
		}
	    }
	  else
	    conn[client].itimeout = WAITTIME;
	}
      return;
    }
  if (conn[client].timeout)
    {
      conn[client].timeout--;
      if (!conn[client].timeout)
	{
	  sflushreliable (client);
	  conn[client].timeout = WAITTIME;
	}
    }
  if (!conn[client].opos)
    return;
  PUTLONG (conn[client].obuffer, conn[client].ircount);
  /*if (!(rand()%10)) *//*emulate internet..send every 10th packet :))) */
  conn[client].failed = 0;
  if ((result = DgramSend (conn[client].socket, conn[client].hostname, conn[client].port,
		(char *) conn[client].obuffer, conn[client].opos + 4)) <= 0)
    {
      if (errno == EWOULDBLOCK || errno == EAGAIN || result >= 0)
	{
	  if (conn[client].timeout == WAITTIME)
	    conn[client].timeout = 1;
	  GetSocketError (conn[client].socket);
	  conn[client].failed = 1;
	}
      else
	{
	  perror ("Socket error");
	  Quit ("Server error:can not send message to client ");
	}
    }
  packets++;
  size += conn[client].opos;
  conn[client].opos = 0;
}

void
ssendbuffers ()
{
  int             i;
  for (i = 0; i <
       connected; i++)
    ssendbuffer (i);
}
void
setrrcount (int i, long scount)
{
  int             size, pos = 0, count;
  if (scount <= conn[i].rrcount)
    return;			/*Old repeated message */
  do
    {
      GETLONG ((conn[i].rbuffer + pos), count);
      GETSHORT ((conn[i].rbuffer + pos) + 4, size);
      pos += size;
    }
  while (count < scount);
  memmove (conn[i].rbuffer, conn[i].rbuffer + pos, conn[i].rpos - pos);
  conn[i].rpos -= pos;
  conn[i].rrcount = scount;
}
void
ssendreliable (int client, unsigned char *message, int size)
{
  if (size == 0)
    return;
  if (conn[client].rpos + size + 6 > BUFFERSIZE)
    {
      Quit ("Connection to client broken too long! Reliable buffer owerfllow\n");
    }

  PUTLONG ((conn[client].rbuffer + conn[client].rpos), conn[client].rcount);
  conn[client].rcount++;
  PUTSHORT ((conn[client].rbuffer + conn[client].rpos + 4), (size + 6));
  memcpy (conn[client].rbuffer + conn[client].rpos + 6, message, size);
  conn[client].rpos += size + 6;
  sizes[message[0]] += size;
  conn[client].timeout = 1;
}
void
ssend (int client, unsigned char *message, int size)
{
  if (conn[client].opos + size + 6 > BUFFERSIZE)
    ssendbuffer (client);

  PUTSHORT ((conn[client].obuffer + conn[client].opos + 4), size + 2);
  memcpy (conn[client].obuffer + conn[client].opos + 6, message, size);
  conn[client].opos += size + 2;
  sizes[message[0]] += size;

}
void
ssendbegining (int client, unsigned char *message, int size)	/*send at more reliable beggining of packet */
{
  if (conn[client].opos + size + 6 > BUFFERSIZE)
    ssendbuffer (client);

  memmove (conn[client].obuffer + size + 6, conn[client].obuffer + 4, conn[client].opos);
  PUTSHORT ((conn[client].obuffer + 4), size + 2);
  memcpy (conn[client].obuffer + 6, message, size);
  conn[client].opos += size + 2;
  sizes[message[0]] += size;

}
void
ssendallr (unsigned char *message, int size)
{
  int             n;
  for (n = 0; n < connected; n++)
    {
      if (conn[client].ready)
	ssend (n, message, size);
    }
}
void
ssendallreliable (unsigned char *message, int size)
{
  int             n;
  for (n = 0; n < connected; n++)
    {
      ssendreliable (n, message, size);
    }
}
void
ssendall (unsigned char *message, int size)
{
  int             n;
  for (n = 0; n < connected; n++)
    {
      ssend (n, message, size);
    }
}
void
Quit (char *text)
{
  static int      quited = 0;
  static int      n;
  if (quited)
    return;
  quited = 1;
  PUTHEAD (CQUITS);
  strcpy ((char *) (buffer + HEADSIZE), text);
  for (n = 0; n < 5; n++)
    {				/*repeat quiting messages to make sure that
				   all clients received it */
      ssendall (buffer, strlen (text) + HEADSIZE + 1);
      ssendbuffers ();
    }
  closeSockets ();
  printf ("Game tarminated.\nReason:%s\n", text);
  exit (0);
}
int
PlaySound (int s)
{
  PUTHEAD (CSOUND);
  PUTCHAR ((buffer + HEADSIZE), s);
  ssendallr (buffer, HEADSIZE + 1);
  return 0;
}
void
Explosion (int x, int y, int type, int letter, int n)
{
  PUTHEAD (CEXPLOSSION);
  PUTSHORT ((buffer + HEADSIZE), x);
  PUTSHORT ((buffer + HEADSIZE + 2), y);
  PUTCHAR ((buffer + HEADSIZE + 4), type);
  PUTCHAR ((buffer + HEADSIZE + 5), letter);
  PUTCHAR ((buffer + HEADSIZE + 6), n);
  ssendallr (buffer, HEADSIZE + 7);
}
void
Outro ()
{
  int             i;
  PUTHEAD (COUTRO);
  ssendallreliable (buffer, HEADSIZE);
  for (i = 0; i < connected; i++)
    conn[client].ready = 0;
}
void
CreatorsPoints (int radius, int x, int y, int color)
{
  PUTHEAD (CCREATOR);
  PUTSHORT ((buffer + HEADSIZE), x);
  PUTSHORT ((buffer + HEADSIZE + 2), y);
  PUTSHORT ((buffer + HEADSIZE + 4), radius);
  PUTCHAR ((buffer + HEADSIZE + 6), color);
  ssendallr (buffer, HEADSIZE + 7);
}

static int
contact ()
{
  int             bytes;
  int             sock, my_port;
  int             accept = 1;
  char           version = 1;
  if (connected >= 6)
    {
      printf ("Maximum number of clients reached!\n");
      return 0;
    }
  if ((bytes = DgramReceiveAny (Socket, (char *) buffer, BUFFERSIZE)) <= 3)
    {
      if (bytes >= 0
	  || errno == EWOULDBLOCK
	  || errno == EAGAIN
	  || errno == EINTR)
	{
	  /*
	   * This caused some long series of error messages
	   * if a player connection crashed violently (SIGKILL, SIGSEGV).
	   * error("SocketRead (pack from %s)", DgramLastaddr());
	   */
	  /*
	   * Clear the error condition for the contact socket.
	   */
	  GetSocketError (Socket);
	  return 0;
	}
      /*printf ("******************SERVER CRASH***********************\n"
         "This port is dangorous-someone attacking me\n"
         "Try another\nmaybe only internet noise?\n"); */
      return 0;
    }
/*It is client's connection message? */
  if (memcmp ("Koules", buffer, 7) || bytes != INITPACKETSIZE)
    {
/*      printf ("****WARNING****WARNING****WARNING****WARNING****\n"
   "Some saboteur atacked my port!\n"
   "Transaction can not be completted\n"
   "Good bye :)\n"); */
	return 0;
    }
  printf ("Client contacted me!\n");
  {
    int             x;
    GETSHORT (buffer + 8, x);
    if (x < GAMEWIDTH)
      accept = 0, printf ("Client does not support %i game width(supports only:%i)\n", x, GAMEWIDTH);
    GETSHORT (buffer + 10, x);
    if (x < GAMEHEIGHT)
      accept = 0, printf ("Client does not support %i game height(supports only:%i)\n", x, GAMEHEIGHT);
    if (connected >= 5)
      accept = 0;
  }
/*Is server able to accept it? */
  connp = conn + connected;
  connp->version = buffer[8];
  strcpy (connp->hostname, DgramLastaddr ());
  GETCHAR (buffer + 7, connp->version);
  /*SetTimeout(1,10000); */
  GETLONG (buffer + 12, connp->magic);
  {
    int             i;
    for (i = 0; i < connected; i++)
      {
	if (conn[i].magic == connp->magic && !strcmp (conn[i].hostname, connp->hostname))
	  return 0;
	/* We've received old repeated message */
      }
  }
  connp->rcount = 1;
  if ((sock = CreateDgramSocket (0)) == -1)
    {
      error ("Cannot create datagram socket ");
      connp--;
      return 0;
    }

  /*if (sock >= MAX_SELECT_FD)
     {
     errno = 0;
     error ("Socket filedescriptor too big");
     close (sock);
     connp--;
     return 0;
     } */
  if ((my_port = GetPortNum (sock)) == 0)
    {
      error ("Cannot get port from socket");
      close (sock);
      connp--;
      return 0;
    }
  if (!accept)
    my_port = 0;
  if (SetSocketNonBlocking (sock, 1) == -1)
    {
      error ("Cannot make client socket non-blocking");
      close (sock);
      connp--;
      return 0;
    }
  if (SetSocketReceiveBufferSize (sock, NETBUFFER) == -1)
    {
      printf ("Cannot set receive buffer size to %d", NETBUFFER);
      connp--;
      close (sock);
      return 0;
    }
  if (SetSocketSendBufferSize (sock, NETBUFFER) == -1)
    {
      printf ("Cannot set send buffer size to %d", NETBUFFER);
      connp--;
      close (sock);
      return 0;
    }
  connp->socket = sock;
  connp->version = version;
  connp->port = DgramLastport ();
  connp->itimeout = WAITTIME;
  printf ("Client #%i:\n", connected);
  printf ("port:%i\n", my_port);
  printf ("address:%s\n", connp->hostname);
  printf ("lastport:%i\n", connp->port);
  PUTLONG (connp->initmessage, my_port);
  PUTSHORT ((connp->initmessage) + 4, GAMEWIDTH);
  PUTSHORT ((connp->initmessage) + 6, GAMEHEIGHT);
  GetSocketError(sock);
  if (DgramSend (Socket , DgramLastaddr (), DgramLastport (),
	    (char *) connp->initmessage, sizeof (connp->initmessage)) == -1)
    {
      if (errno != EWOULDBLOCK && errno != EAGAIN)
	{
	  error ("Cant contact client\n");
	  connp--;
	  close (sock);
	  return 0;
	}
      GetSocketError (sock);
    }
  if (!accept)
    {
      printf ("client not accepted. sorry\n");
      connp--;
      close (sock);
      return 0;
    }
  connected++;
  return 1;
}
static void
client_message (struct conn *c)
{
  long            scount;
  int             bytes, pos = 4;
  GetSocketError (c->socket);
  if ((bytes = (c->connected ? DgramReceiveConnected (c->socket, (char *) ibuffer, BUFFERSIZE)
		: DgramReceiveAny (c->socket, (char *) ibuffer, BUFFERSIZE))
      ) <= 4)
    {
      if (errno == EWOULDBLOCK || errno == EAGAIN || bytes >= 0)
	{
	  GetSocketError (c->socket);
	  return;
	}
      perror ("Socket error");
      GetSocketError (c->socket);
      Quit ("Connection to client broken-exiting game\n");
    }
  isize += bytes;
  GETLONG (ibuffer, scount);
  setrrcount ((int) (c - conn), scount);
  ipackets++;
  while (pos + 2 < bytes)
    {
      int             size;
      GETSHORT (ibuffer + pos, size);
      if (pos + size <= bytes)
	{
	  isizes[ibuffer[pos + 2]]++;
	  stable[ibuffer[pos + 2]].func ((int) (c - conn), (ibuffer + pos + 3), size - 3);
	}
      pos += size;
    }
}
void
wposs ()
{
  int             size;
  int             i;
  PUTHEAD (CPOS);
  size = write_objects (buffer + 1);
  for (i = 0; i < connected; i++)
    {
      conn[client].sent = 0;
      if ( /*conn[client].wpos && */ (conn[client].dispframes >
			 conn[client].sentframes && conn[client].ready) || (
				  conn[client].failed && conn[client].sent))
	{
	  conn[client].counter += conn[client].dispframes;
	  if ((conn[client].counter > conn[client].allframes) ||
	      (conn[client].failed && conn[client].sent))
	    {
	      conn[client].sent = 1;
	      conn[client].sentframes++;
	      conn[client].counter -= conn[client].allframes;
	      ssend (i, buffer, size + HEADSIZE);
	    }
	}
    }
  for (i = 0; i < 5; i++)
    acceled[i] = 0;
}

static void
exitcall ()
{
  Quit ("Server called exit!");
}
static void
interrupt (int i)
{
  char            s[256];
  sprintf (s, "Server received interrupt %i\n", i);
  Quit (s);
}
extern void     server_loop2 (void);

void
server_loop ()
{
  long            VfTime = 0;
  long            VendSleep = 0;
  struct timeval  VlastClk;
  struct timeval  VnewClk;
  struct conn    *c;
  int             ecounter = 0;
  int             wait = 0;
  server = 1;

  gettimeofday (&VlastClk, NULL);
  gettimeofday (&VnewClk, NULL);
  VendSleep = VlastClk.tv_usec;
  VfTime = 1000000 / 25;
  SetTimeout (0, 10 * 1000);
  printf ("Awaiting clients....\n");
  while (!SocketReadable (Socket) || !contact ())
    SetTimeout (0, 10 * 1000);
  SetTimeout (0, 0);
  printf ("First client connected-starting setup part of game\n");
  atexit (exitcall);
  signal (SIGHUP, interrupt);
  signal (SIGINT, interrupt);
  signal (SIGTRAP, interrupt);
  signal (SIGABRT, interrupt);
  signal (SIGSEGV, interrupt);
  signal (SIGQUIT, interrupt);
  signal (SIGFPE, interrupt);
  signal (SIGTERM, interrupt);
  signal (SIGBUS, interrupt);
  signal (SIGIOT, interrupt);
  signal (SIGILL, interrupt);
  nrockets = 0;
  gamemode = MENU;
  while (1)
    {
      ecounter--;
      if (ecounter < 0)
	{
	  ecounter = 15, extesions ();
	}
      frames++;
      SetTimeout (0, 0);
      if (Socket != -1)
	while (SocketReadable (Socket))
	  {
	    contact ();
	  }
      for (c = &conn[0]; c <= connp; c++)
	{
	  while (SocketReadable (c->socket))
	    {
	      client_message (c);
	    }
	}
      gettimeofday (&VnewClk, NULL);
      if (VnewClk.tv_usec < VendSleep)
	VendSleep -= 1000000;
      wait = (VfTime - VnewClk.tv_usec + VendSleep);
      /* if (tbreak) */
      wait = VfTime /*, tbreak = 0 */ ;
      tbreak = 0;
      if (gamemode != PREGAME)
	server_loop2 ();
      wposs ();
      ssendbuffers ();
      do
	{
	  gettimeofday (&VnewClk, NULL);
	  if (VnewClk.tv_usec < VendSleep)
	    VendSleep -= 1000000;
	  wait = (VfTime - VnewClk.tv_usec + VendSleep);
	  if (tbreak)
	    wait = VfTime;
	  if (wait > 0)
	    usleep (wait);
	}
      while (wait > 10 && !tbreak);
      VendSleep = VnewClk.tv_usec + wait;
      gettimeofday (&VlastClk, NULL);
      if (tbreak)
	VendSleep = VlastClk.tv_usec;
      tbreak = 0;

    }
}
void
init_server (void)
{
  if ((Socket = CreateDgramSocket (initport)) == -1)
    {
      error ("Could not create Dgram socket");
      exit (-1);
    }
  printf ("Initializing koules network server at port: %i\n", initport);
  if (SetSocketNonBlocking (Socket, 1) == -1)
    {
      error ("Can't make contact socket non-blocking");
      exit (-1);
    }

  SetTimeout (0, 0);
}
#endif
