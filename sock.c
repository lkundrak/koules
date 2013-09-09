/* This is mostly emulation of socklib.c from xpilot used by koules before.
 * It was removed because of copyright problems. Current implementation is
 * completly rewrote just partially compatible with socklib API. It implements
 * just few calls used by koules and in a bit different way. It is not drop-in
 * replacement for socklib.
 */
#if defined(__sparc)
#define __EXTENSIONS__
#endif
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <sys/ioctl.h>
#if (SVR4)
#include <sys/filio.h>
#endif
#ifdef __hpux
#include <time.h>
#else
#include <sys/time.h>
#endif
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include <signal.h>
#include <setjmp.h>
#include <errno.h>
#if defined(__sparc)
#include <sys/fcntl.h>
#endif
#if defined(__sun__)
#include <arpa/nameser.h>
#include <resolv.h>
#endif

#include "sock.h"
static struct timeval timeout;
static struct sockaddr_in lastaddr;


void SetTimeout(int s,int us)
{
  timeout.tv_sec=s;
  timeout.tv_usec=us;
}

int GetPortNum(int fd)
{
  struct sockaddr_in a;
  int s=sizeof(a);
  if(getsockname(fd,(struct sockaddr *)&a,&s)<0) return -1;
  return(ntohs(a.sin_port));
}
char *GetSockAddr(int fd)
{
  struct sockaddr_in a;
  int s=sizeof(a);
  if(getsockname(fd,(struct sockaddr *)&a,&s)<0) return NULL;
  return(inet_ntoa(a.sin_addr));
}
int SetSocketReceiveBufferSize(int fd,int size)
{
	return(setsockopt(fd, SOL_SOCKET, SO_RCVBUF, (void *)&size, sizeof(size)));
}
int SetSocketSendBufferSize(int fd,int size)
{
	return(setsockopt(fd, SOL_SOCKET, SO_SNDBUF, (void *)&size, sizeof(size)));
}


int SetSocketNonBlocking (int fd, int flag)
{
/*
 * This code is copied from original socklib library. I hope it is not
 * problem... I want to avoid compilation problems on some systems...
 *
 * There are some problems on some particular systems (suns) with
 * getting sockets to be non-blocking.  Just try all possible ways
 * until one of them succeeds.  Please keep us informed by e-mail
 * to xpilot@cs.uit.no.
 */

#ifndef USE_FCNTL_O_NONBLOCK
#ifndef USE_FCNTL_O_NDELAY
#ifndef USE_FCNTL_FNDELAY
#ifndef USE_IOCTL_FIONBIO

#if defined(_SEQUENT_) || defined(__svr4__) || defined(SVR4)
#define USE_FCNTL_O_NDELAY
#elif defined(__sun__) && defined(FNDELAY)
#define USE_FCNTL_FNDELAY
#elif defined(FIONBIO)
#define USE_IOCTL_FIONBIO
#elif defined(FNDELAY)
#define USE_FCNTL_FNDELAY
#elif defined(O_NONBLOCK)
#define USE_FCNTL_O_NONBLOCK
#else
#define USE_FCNTL_O_NDELAY
#endif

#endif
#endif
#endif
#endif

#ifdef USE_FCNTL_FNDELAY
  if (fcntl (fd, F_SETFL, (flag != 0) ? FNDELAY : 0) != -1)
    return 0;
  fprintf (stderr, "fcntl FNDELAY failed in file \"%s\", line %d: %s\n",
	   __FILE__, __LINE__, strerror (errno));
#endif

#ifdef USE_IOCTL_FIONBIO
  if (ioctl (fd, FIONBIO, &flag) != -1)
    return 0;
  fprintf (stderr, "ioctl FIONBIO failed in file \"%s\", line %d: %s\n",
	   __FILE__, __LINE__, strerror (errno));
#endif

#ifdef USE_FCNTL_O_NONBLOCK
  if (fcntl (fd, F_SETFL, (flag != 0) ? O_NONBLOCK : 0) != -1)
    return 0;
  fprintf (stderr, "fcntl O_NONBLOCK failed in file \"%s\", line %d: %s\n",
	   __FILE__, __LINE__, strerror (errno));
#endif

#ifdef USE_FCNTL_O_NDELAY
  if (fcntl (fd, F_SETFL, (flag != 0) ? O_NDELAY : 0) != -1)
    return 0;
  fprintf (stderr, "fcntl O_NDELAY failed in file \"%s\", line %d: %s\n",
	   __FILE__, __LINE__, strerror (errno));
#endif

  return (-1);
}	

int GetSocketError(int fd)
{
	int size=sizeof(errno);
	return (getsockopt(fd,SOL_SOCKET,SO_ERROR,(char *)&errno,&size));
}

int SocketReadable(int fd)
{
	fd_set fds;
        FD_ZERO(&fds);
	FD_SET(fd,&fds);
	if (select (fd + 1, &fds, NULL, NULL, &timeout) == -1) return ((errno == EINTR) ? 0 : -1);
	return(FD_ISSET(fd,&fds));
}

int CreateDgramSocket(int port)
{
        struct sockaddr_in addr;
	int fd;

	fd=socket(AF_INET, SOCK_DGRAM, 0);
	if (fd<0) return -1;
	memset(&addr,0,sizeof(addr));
	addr.sin_addr.s_addr=INADDR_ANY;
	addr.sin_port=htons(port);
	addr.sin_family=AF_INET;
	if(bind(fd,(struct sockaddr *)&addr,sizeof(struct sockaddr_in))<0) {
			fd=errno;
			close(fd);
			errno=fd;
			return -1;
			}
        return fd;
	  
}
int DgramConnect(int fd, char *host, int port)
{
	struct sockaddr_in addr;
	memset(&addr,0,sizeof(addr));
	addr.sin_addr.s_addr = inet_addr (host);
	if(addr.sin_addr.s_addr==-1) return -1;
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
	return(connect (fd, (struct sockaddr *) &addr, sizeof (addr)));
}
int DgramSend(int fd, char *host, int port, char *sbuf, int size)
{
	struct sockaddr_in addr;
	struct hostent *hp;
#if 0
	if (!rand()%10) return 0;/*Internet emulation :))))))) */
	if (rand()%2) {size=rand()%size;}
#endif
	memset(&addr,0,sizeof(addr));
	addr.sin_addr.s_addr = inet_addr (host);
	if(addr.sin_addr.s_addr==-1) {
		if (addr.sin_addr.s_addr == (int) -1)
		{
			hp = gethostbyname (host);
			if (hp == NULL)
			{
				fprintf(stderr,"Host '%s' unknown\n",host);
				return -1;
			}
			else
				addr.sin_addr.s_addr = ((struct in_addr *) (hp->h_addr))->s_addr;
		}

	}
	addr.sin_family = AF_INET;
	addr.sin_port = htons (port);
        return(sendto (fd, sbuf, size, 0, (struct sockaddr *) &addr,
		                     sizeof (struct sockaddr_in)));

}
int DgramReceiveAny(int fd,char *buf, int size)
{
	int len=sizeof (struct sockaddr_in);
	return(recvfrom (fd, buf, size, 0, (struct sockaddr *) &lastaddr, &len));
}
int DgramReceiveConnected(int fd,char *buf, int size)
{
	return(recv (fd, buf, size, 0));
}
int SocketClose (int fd)
{
       shutdown (fd, 2);
       if (close (fd) == -1)
          {
	       return (-1);
          }
     return (1);
}
char *DgramLastaddr (void)
{
	  return (inet_ntoa (lastaddr.sin_addr));
}
int DgramLastport (void)
{
	  return (ntohs ((int) lastaddr.sin_port));
} 


