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
*                     licence details.                     *
*----------------------------------------------------------*
*  interface.h   interface for vgakeyboard                 *
***********************************************************/
#define SCANCODE_P 25
struct keyrec
  {
    int             scancode;
    struct keyrec  *next;
    struct keyrec  *last;
  };
#ifdef PLATFORM_VARIABLES_HERE
#define where
#else
#define where extern
#endif
#define MOUSE
where struct keyrec *ffirstkey
#ifdef PLATFORM_VARIABLES_HERE
=NULL
#endif
;
where int       penter, pup, pdown,ph, pp, pesc, pleft, pright;
where int       mousex, mousey;
where unsigned int mousebuttons;
#undef where
#define UpdateInput() ProcessEvents();
#define IsPressed(scancode) (findkey(scancode)!=NULL?1:0)
#define MouseButtons() mousebuttons
#define MouseX() mousex
#define MouseY() mousey

#define Pressed() (ffirstkey!=NULL?1:mousebuttons&&mousex>0&&mousey>0&&mousex<MAPWIDTH&&mousey<MAPHEIGHT)

#define IsPressedEsc() pesc

#define IsPressedP() pp
#define IsPressedH() ph

#define IsPressedEnter() penter

#define IsPressedUp() pup

#define IsPressedDown() pdown

#define IsPressedLeft() pleft

#define IsPressedRight() pright
extern int      GetKey ();
extern struct keyrec *findkey (int);
extern void     addkey (int);
extern void     deletekey (int);
extern void     ProcessEvents ();
