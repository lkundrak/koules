/***********************************************************
*                      K O U L E S                         *
*----------------------------------------------------------*
*  C2013 Lubomir Rintel                                    *
*        Polivkova 10                                      *
*        612 00 Brno                                       *
*        Czech Republic                                    *
*        Phone: 0041-739-669-116                           *
*        eMail: lkundrak@v3.sk                             *
*----------------------------------------------------------*
*   Copyright(c)2013 by Lubomir Rintel.See README for      *
*                     Liecnece details.                    *
*----------------------------------------------------------*
*  sdl/interface.h common definitions for SDL backend      *
***********************************************************/

#ifndef _KOULES_SDL_INTERFACE_H
#define _KOULES_SDL_INTERFACE_H

#include <stdbool.h>
#include <SDL.h>

typedef struct {
	SDL_Surface *surface;
	SDL_Renderer *renderer;
} BitmapType, RawBitmapType, VScreenType;

extern SDL_Surface *sdl_screen;
extern SDL_Window *sdl_window;

#define COLORS 256
typedef struct
{
  struct
  {
    Uint8           red;
    Uint8           green;
    Uint8           blue;
  }
  color[COLORS];
}
Palette;

extern VScreenType background;
extern VScreenType backscreen;
extern VScreenType starbackground;

extern int      GAMEWIDTH;
extern int      GAMEHEIGHT;
extern int      MAPWIDTH;
extern int      MAPHEIGHT;
extern int      DIV;

#define EYE_RADIUS (DIV==1?5:6)
#define MOUSE_RADIUS 4

RawBitmapType   CreateBitmap (const int, const int);
BitmapType      CompileBitmap (const int, const int, const RawBitmapType);
void            ClearScreen (void);
void            SetScreen (VScreenType screen);
void            CopyToScreen (VScreenType);
void            CopyVSToVS (VScreenType, VScreenType);

void            UpdateInput (void);
int             GetKey (void);
bool            Pressed (void);
bool            IsPressed (int);
int             IsPressedDown (void);
int             IsPressedEnter (void);
int             IsPressedEsc (void);
int             IsPressedH (void);
int             IsPressedLeft (void);
int             IsPressedP (void);
int             IsPressedRight (void);
int             IsPressedUp (void);

void            BSetPixel (RawBitmapType bitmap, int, int, int);
void            SMySetPixel (VScreenType, int, int, int);
int             SGetPixel (int, int);
void            SPutPixel (int, int, int);
void            SSetPixel (int, int, int);
void            DrawText (int, int, char *);
void            DrawBlackMaskedText (int, int, char *);
void            DrawWhiteMaskedText (int, int, char *);
void            DrawRectangle (int, int, int, int, int);
void            HLine (int, int, int, int);
void            Line (int, int, int, int, int);
void            Line1 (int, int, int, int, int);
void            PutBitmap (const int, const int, const int, const int,
			   const BitmapType);

void            EnableClipping (void);
void            DisableClipping (void);

void            WaitRetrace (void);
void            SetPalette (Palette * pal);

void            myusleep (unsigned long);
void            uninitialize (void);

void            fadeout (void);
void            fadein (void);
void            fadein1 (void);

#ifdef MOUSE
int             MouseX (void);
int             MouseY (void);
int             MouseButtons (void);
#endif

#endif /* _KOULES_SDL_INTERFACE_H */
