#ifdef NETSUPPORT
#ifndef SERVER_H
#define SERVER_H
void            init_server (void);
void            server_loop (void);
void            ssend (int client, unsigned char *message, int size);
void            ssendbegining (int client, unsigned char *message, int size);
void            ssendall (unsigned char *message, int size);
void            ssendallr (unsigned char *message, int size);
void            ssendreliable (int client, unsigned char *message, int size);
void            ssendallreliable (unsigned char *message, int size);
void            Quit (char *text);
void            SEffect (int n, int nos);
void            Outro ();
void            Explosion (int x, int y, int type, int letter, int n);
void            CreatorsPoints (int radius, int x, int y, int color);
/*extern struct control
   {
   int             type;
   double          jx, jy;
   int             mx, my;
   int             mask;
   }
   controls[5]; */
extern char     acceled[5];
extern int      serverstartlevel;
extern int      serverdifficulty;
extern int      servergameplan;
#endif
#endif
