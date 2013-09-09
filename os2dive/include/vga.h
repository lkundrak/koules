#ifndef _VGA_H_
#define _VGA_H_

#ifndef __OS2__
#define __OS2__
#endif

#ifdef __cplusplus
extern "C"
{
#endif

#define INCL_DOS
#define INCL_GPI
#define INCL_WIN
#include <os2.h>
#define  _MEERROR_H_
#include <mmioos2.h>                   /* It is from MMPM toolkit           */
#include <dive.h>
#include <fourcc.h>

#define TEXT 	     0		/* Compatible with VGAlib v1.2 */
#define G320x200x16  1
#define G640x200x16  2
#define G640x350x16  3
#define G640x480x16  4
#define G320x200x256 5
#define G320x240x256 6
#define G320x400x256 7
#define G360x480x256 8
#define G640x480x2   9

#define G640x480x256 10
#define G800x600x256 11
#define G1024x768x256 12

#define __GLASTMODE G1024x768x256

    extern int vga_setmode(int mode);
    extern int vga_hasmode(int mode);
    extern int vga_setflipchar(int c);

    extern int vga_clear(void);
    extern int vga_flip(void);

    extern int vga_getxdim(void);
    extern int vga_getydim(void);
    extern int vga_getcolors(void);

    extern int vga_setpalette(int index, int red, int green, int blue);
    extern int vga_getpalette(int index, int *red, int *green, int *blue);
    extern int vga_setpalvec(int start, int num, int *pal);
    extern int vga_getpalvec(int start, int num, int *pal);

    extern int vga_screenoff(void);
    extern int vga_screenon(void);

    extern int vga_setcolor(int color);
    extern int vga_drawpixel(int x, int y);
    extern int vga_drawline(int x1, int y1, int x2, int y2);
    extern int vga_drawscanline(int line, unsigned char *colors);
    extern int vga_drawscansegment(unsigned char *colors, int x, int y, int length);
    extern int vga_getpixel(int x, int y);	/* Added. */

    extern int vga_getch(void);

    extern int vga_dumpregs(void);

/* Window data structure */
typedef struct _WINDATA
{
    BOOL   fVrnDisabled;             /* ????  Visual region enable/disable  */
    BOOL   fChgSrcPalette;           /* Flag for change of source palette   */
    BOOL   fModeChange;              /* Switching to FSDive?                */
    HAB    hab;
    HWND   hwndMenu;                 /* User-menu window handle             */
    HWND   hwndFrame;                /* Frame window handle                 */
    HWND   hwndClient;               /* Client window handle                */
    HDIVE  hDive;                    /* DIVE handle                         */
    ULONG  ulWidth;                  /* Bitmap width in pels                */
    ULONG  ulHeight;                 /* Bitmap Height in pels               */
    FOURCC fccColorFormat;           /* Bitmap color format                 */
    PBYTE  OffScrBuffer;             /* Pointer to off-screen buffer        */
    ULONG  ulColors;                 /* Number of colors                    */
    TID    tidBlitThread;            /* Thread ID for blitting routine      */
    ULONG  ulSrcLineSizeBytes;       /* source linesize                     */
} WINDATA, *PWINDATA;

typedef struct {
    int width;
    int height;
    int bytesperpixel;
    int colors;
    int linewidth;		/* scanline width in bytes */
    int maxlogicalwidth;	/* maximum logical scanline width */
    int startaddressrange;	/* changeable bits set */
    int maxpixels;		/* video memory / bytesperpixel */
    int haveblit;		/* mask of blit functions available */
    int flags;		        /* other flags */
    
} vga_modeinfo;

    extern vga_modeinfo *vga_getmodeinfo(int mode);
    extern int vga_getdefaultmode(void);
    extern int vga_getcurrentmode(void);
    extern int vga_getcurrentchipset(void);
    extern char *vga_getmodename(int mode);
    extern int vga_getmodenumber(char *name);
    extern int vga_lastmodenumber(void);

    extern unsigned char *graph_mem;
    extern unsigned char *vga_getgraphmem(void);

    extern void vga_setpage(int p);
    extern void vga_setreadpage(int p);
    extern void vga_setwritepage(int p);
    extern void vga_setlogicalwidth(int w);
    extern void vga_setdisplaystart(int a);
    extern void vga_waitretrace(void);
    extern int vga_claimvideomemory(int n);
    extern void vga_disabledriverreport(void);
    extern int vga_setmodeX(void);
    extern int vga_init(void);	/* Used to return void in svgalib <= 1.12. */
    extern int vga_getmousetype(void);
    extern int vga_getmonitortype(void);
    extern void vga_setmousesupport(int s);
    extern void vga_lockvc(void);
    extern void vga_unlockvc(void);
    extern int vga_getkey(void);
    extern void vga_runinbackground(int s);
    extern int vga_oktowrite(void);
    extern void vga_copytoplanar256(unsigned char *virtualp, int pitch,
				  int voffset, int vpitch, int w, int h);
    extern void vga_copytoplanar16(unsigned char *virtualp, int pitch,
				   int voffset, int vpitch, int w, int h);
    extern void vga_copytoplane(unsigned char *virtualp, int pitch,
		       int voffset, int vpitch, int w, int h, int plane);
    extern int vga_setlinearaddressing(void);
    extern void vga_safety_fork(void (*shutdown_routine) (void));

/* Special OS/2 extensions */
extern PWINDATA _pwinData;
extern ULONG ulImage[1];
extern DIVE_CAPS DiveCaps;

extern int vga_Diveinit(void);
extern void vga_wm_create(HWND hwnd, HAB hab);
extern int vga_wm_vrnenabled(HWND hwnd);
extern int vga_wm_realizepalette(HWND hwnd);
extern int vga_wm_close(void);
extern void vga_os2_DesktopDive(void);
extern void vga_cleanup(void);
extern void vga_initwinpos(void);

extern void vga_pm_sendWMcmd(ULONG parm1, ULONG parm2);

extern void vga_titleprint(char *);
extern void forceBlitting(void);

/* FSDive stuff */

extern void vga_os2_disableFSDive(void);          /* disable FSDive support  */
extern BOOL vga_os2_presentFSDive(void);          /* FSDive support?         */
extern void vga_os2_setFSDiveFullScreen(BOOL m);  /* */
extern BOOL vga_os2_FSDiveFullScreen(void);       /* are we running FullScr? */
extern BOOL _vga_os2_textmode_called;

#define WM_GetVideoModeTable     0x04A2
#define WM_SetVideoMode          0x04A0
#define WM_NotifyVideoModeChange 0x04A1

/* Use this instead of e.g. ID_RESOURCE
 * Remember to #include <vga.h> in .rc or whatever
 */
#define VGA_ID_RESOURCE 1

/* These must be catched in the event-handler */
#define VGA_ID_DESKSCR 502
#define VGA_ID_NEWTEXT 503

/* This must be defined by the user */
extern MRESULT EXPENTRY vgaAppWindowProc(HWND, ULONG, MPARAM, MPARAM);

#define RealScreenX (WinQuerySysValue(HWND_DESKTOP, SV_CXSCREEN))
#define RealScreenY (WinQuerySysValue(HWND_DESKTOP, SV_CYSCREEN))

#endif
