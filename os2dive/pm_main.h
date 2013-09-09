#ifndef _PMMAIN_H_
#define _PMMAIN_H_

/* This still required, I'm sorry :-) */
#define ID_RESOURCE       1

/* Top level menus */
#define IDM_OPTIONS       1
#define IDM_SCREEN        2

/* Sub menus */
#define IDM_SOUND        10
#define IDM_EXIT         11

#define IDM_FULLSCR      20
#define IDM_DESKSCR      21
#define IDM_SNAP         22
#define IDM_CENTER       23

/* Icons */
#define ID_GREENBALL    16
#define ID_REDBALL      17
#define ID_KOULES       18
#define ID_KOULES2      19
#define ID_CURSOR       20

/* Dialog things */
#define IDD_DLGBOX                  1

#define IDD_320x200                 200
#define IDD_320x240                 201 
#define IDD_640x480                 202

#define IDD_COLORS                  300
#define IDD_MONO                    301
#define IDD_BW                      302
#define IDD_SOUND                   303
#define IDD_MOUSE                   304

/* For FSDive */
#define WS_DesktopDive    0x00000000L /* Desktop dive window style           */
#define WS_MaxDesktopDive 0x00000001L /* Maximized desktop dive window style */
#define WS_FullScreenDive 0x00000002L /* Full-screen 320x200x256 dive style  */


/* Misc. usefull things */
#define GetSnapWidth(x) ((x)+2*WinQuerySysValue(HWND_DESKTOP,SV_CXSIZEBORDER))
#define GetSnapHeight(x) ((x)\
+2*WinQuerySysValue(HWND_DESKTOP, SV_CYSIZEBORDER)\
+2*WinQuerySysValue(HWND_DESKTOP, SV_CYBORDER) \
+WinQuerySysValue(HWND_DESKTOP, SV_CYMENU) \
+WinQuerySysValue(HWND_DESKTOP, SV_CYTITLEBAR))

#endif
