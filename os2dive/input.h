/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C1995 JAHUSOFT                                          *
*        Jan Hubicka                                       *
*        Dukelskych Bojovniku 1944                         *
*        390 03 Tabor                                      *
*        Czech Republic                                    *
*        Telefon: (048-I think) (0361) 32613               *
*        eMail: hubicka@limax.paru.cas.cz                  *
*----------------------------------------------------------*
* Copyright(c)1995,96 by Jan Hubicka &                     *
*              Thomas A. K. Kjaer See README for license   *
*                             details.                     *
*----------------------------------------------------------*
* intput.h input for OS/2 Warp 3.0 & DIVE                  *
***********************************************************/
struct keyrec
{
    BYTE scancode;
    struct keyrec *next;
    struct keyrec *last;
};

struct keyrec *firstkey;
BYTE penter, pup, pdown, pp, pesc, pleft, pright;
ULONG mousex, mousey, mousebuttons;

#define UpdateInput()
#define IsPressed(scancode) (findkey(scancode)!=NULL?1:0)
#define MouseButtons() mousebuttons
#define MouseX() mousex
#define MouseY() mousey

#define Pressed() (firstkey!=NULL?1:0)

#define IsPressedEsc() pesc

#define IsPressedP() pp

#define IsPressedEnter() penter

#define IsPressedUp() pup

#define IsPressedDown() pdown

#define IsPressedLeft() pleft

#define IsPressedRight() pright

extern int GetKey();
extern struct keyrec *findkey (int);
extern void addkey(int);
extern void deletekey(int);


