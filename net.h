#ifdef NETSUPPORT
#ifndef NET_H
#define NET_H
/*#define PUTLONG(data,long) memcpy(data,&long,4);
   #define GETLONG(data,long) memcpy(&long,data,4); */
/*my own ones-faster and shorter than calling library routines..
   why gcc don't inline them? */
#define PUTCHAR(data,long) *(unsigned char *)(data)=(unsigned char)(long);
#define GETCHAR(data,long) (long)=*(unsigned char *)(data);
#define PUTSHORT(data,long) *(unsigned char *)(data)=(unsigned short)(long)&0xff,*((unsigned char *)(data)+1)=((long)>>8)&0xff;
#define GETSHORT(data,long) long=*(unsigned char *)(data),long+=*((unsigned char *)data+1)<<8;
#define PUTLONG(data,long) *(unsigned char *)(data)=(unsigned short)(long)&0xff,*((unsigned char *)(data)+1)=((long)>>8)&0xff\
                                                                               ,*((unsigned char *)(data)+2)=((long)>>16)&0xff\
                                                                               ,*((unsigned char *)(data)+3)=((long)>>24)&0xff;
#define GETLONG(data,long) long=*(unsigned char *)(data),long+=*((unsigned char *)data+1)<<8;\
                                                         long+=*((unsigned char *)data+2)<<16;\
                                                         long+=*((unsigned char *)data+2)<<24;
#define HEADSIZE 1
/* Client messages */
#define CPRINT 0
#define CQUITS 1
#define CPOS 2
#define CEXPLOSSION 3
#define CCREATOR 4
#define CSOUND 5
#define CPLAYERS 6
#define CREG 7
#define CMENU2 8
#define CLEVEL 9
#define CGAME 10
#define CRELIABLE 11
#define COUTRO 12
/* Server messages */
#define SPRINT 0
#define SQUIT 1
#define SPOS 2
#define SREGISTER 3
#define SSTART 4
#define SREADY 5
#define SJOYSTICK 6
#define SMOUSE 7
#define SROT 8
#define SKEY 9
#define SRELIABLE 10
#define SINIT 11
#define PUTHEAD(long) PUTCHAR(buffer,long);
#define NETBUFFER (256*16)
/*#define INITPACKETSIZE 17 */
#define INITPACKETSIZE 16
#define REPLYSIZE 8
#define DEFAULTINITPORT 12345
extern int      initport;
#endif
#endif
