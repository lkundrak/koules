
#ifndef CLIENT_H
#define CLIENT_H
extern char     servername[256];
int             init_client (void);
void            client_loop (void);
void            csend (unsigned char *message, int size);
void            start_game ();
void            SendJoystick (int player, int x, int y, int buttons);
void            SendMouse (int player, int x, int y, int buttons);
void            SendKeys (int player, int c);
void            SendRotation (int player, int c);
void            CQuit (char *why);
#endif
