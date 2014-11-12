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
*  sdl/draw.c drawing routines using SDL and SDL_gfx       *
***********************************************************/

#include <interface.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <SDL2_gfxPrimitives.h>

VScreenType     backscreen;
VScreenType     background;
VScreenType     starbackground;

VScreenType     current;
Uint32          palette[COLORS];
bool            clipping;

/* Allocate surface for sprites (balls). */
RawBitmapType
CreateBitmap (const int xv, const int yv)
{
  RawBitmapType   bitmap;

  if (sdl_screen)
    {
      /* HW surface should make create_bitmaps () during initialization,
       * but supposedly speeds up subsequent blitting. */
      bitmap.surface = SDL_CreateRGBSurface (0, xv, yv, 32,
				     0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    }
  else
    {
      /* Create a pixmap when we don't have a screen yet. No attempts at
       * matching the pixel format, this is likely a window icon. */
      bitmap.surface = SDL_CreateRGBSurface (SDL_SWSURFACE, xv, yv, 32,
				0x00ff0000, 0x0000ff00, 0x000000ff, 0xff000000);
    }

  if (bitmap.surface == NULL)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      return bitmap;
    }

  bitmap.renderer = SDL_CreateSoftwareRenderer(bitmap.surface);
  if (bitmap.renderer == NULL)
    {
      fprintf (stderr, "%s\n", SDL_GetError ());
      SDL_FreeSurface (bitmap.surface);
      bitmap.surface = NULL;
      return bitmap;
    }

  return bitmap;
}

/* Do nothing: the bitmaps are pre-rendered on surfaces in blittable form
 * already. */
BitmapType
CompileBitmap (const int x, const int y, const RawBitmapType bitmap)
{
  return bitmap;
}

/* Switch current surface that's being drawn onto. */
void
SetScreen (VScreenType screen)
{
  current = screen;
}

/* Fill screen with solid black. */
void
ClearScreen (void)
{
  SDL_FillRect (current.surface, NULL, SDL_MapRGB (current.surface->format, 0, 0, 0));
}

/* Blit a surface onto physical screen surface and make sure it's
 * rendered. */
void
CopyToScreen (VScreenType source)
{
  SDL_RenderPresent(source.renderer);
  SDL_BlitSurface (source.surface, NULL, sdl_screen, NULL);

  SDL_UpdateWindowSurface(sdl_window);
}

/* Blit a surface onto another one. */
void
CopyVSToVS (VScreenType source, VScreenType destination)
{
  SDL_BlitSurface (source.surface, NULL, destination.surface, NULL);
}

/* Turn on clipping so that out-of-bounds draws are safely discarded.
 * We do not optimize anything for disabled clipping yet. */
void
EnableClipping (void)
{
  clipping = true;
}

/* Turn off clipping to allow for faster rendering when we're sure that
 * there are not out-of-bound draw attempts.
 * We do not optimize anything for disabled clipping yet. */
void
DisableClipping (void)
{
  clipping = false;
}

/* Set a pixel of a ball sprite. c=0 is transparent.
 * Called with clipping both enabled and disabled. */
void
BSetPixel (RawBitmapType bitmap, int x, int y, int c)
{
  if (c)
    pixelColor (bitmap.renderer, x, y, palette[c]);
}

/* Draw a pixel of a player as it builds up in the game area.
 * Called with clipping both enabled and disabled. */
void
SMySetPixel (VScreenType screen, int x, int y, int c)
{
  pixelColor (screen.renderer, x, y >> 8, palette[c]);
}

/* Draw a sprite onto the current screen surface. */
void
PutBitmap (const int x, const int y, const int xsize, const int ysize,
	   const BitmapType bitmap)
{
  SDL_Rect        dst;
  dst.x = x;
  dst.y = y;

  SDL_BlitSurface (bitmap.surface, NULL, current.surface, &dst);
}

/* Get a color of a pixel in current surface. Only used to draw the blue
 * background screen. */
int
SGetPixel (int x, int y)
{
  int             c = 0;
  Uint8           r, g, b;
  Uint32          color;
  Uint32         *pixels = current.surface->pixels;
  SDL_GetRGB (pixels[y * current.surface->w + x], current.surface->format, &r, &g, &b);

  color = 0xff000000;
  color |= r << 0;
  color |= g << 8;
  color |= b << 16;

  do
    {
      if (palette[c] == color)
	return c;
    }
  while (++c < COLORS);

  return 0;
}

/* This is used to draw blue pixels on a background screen.
 * Called with clipping both enabled and disabled. */
void
SPutPixel (int x, int y, int c)
{
  pixelColor (current.renderer, x, y, palette[c]);
}

/* Draw text. Used to draw the "Sector #" message between levels. */
void
DrawText (int x, int y, char *text)
{
  stringColor (current.renderer, x, y, text, 0xffffffff);
}

/* Draw black text. Used to draw shadow in menu text beneath white text. */
void
DrawBlackMaskedText (int x, int y, char *text)
{
  stringColor (current.renderer, x, y, text, 0xff000000);
}

/* Draw white text, on top of black shadow in menu screen and the
 * level/lives counter. */
void
DrawWhiteMaskedText (int x, int y, char *text)
{
  stringColor (current.renderer, x, y, text, 0xffffffff);
}

/* Draw the selection rectangle in menu screen. */
void
DrawRectangle (int x1, int y1, int x2, int y2, int color)
{
  rectangleColor (current.renderer, x1, y1, x2, y2, palette[color]);
}

/* A straight line. Used to delimit menu or play area from the lives/level
 * counter. */
void
Line (int x1, int y1, int x2, int y2, int c)
{
  lineColor (current.renderer, x1, y1, x2, y2, palette[c]);
}

/* Draw a point in the background on the starwars screen.
 * Only called with clipping disabled, coords are safe. */
void
SSetPixel (int x, int y, int c)
{
  pixelColor (current.renderer, x, y, palette[c]);
}

/* Draw thick line with perspective, for starwars screen. */
void
Line1 (int x1, int y1, int x2, int y2, int c)
{
  x1 = (MAPWIDTH / 2 + (x1) * 220 / (1000 - (y1)) / DIV);
  y1 = (int) (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y1)));
  x2 = (int) (MAPWIDTH / 2 + (x2) * 220 / (1000 - (y2)) / DIV);
  y2 = (int) (MAPHEIGHT / 3 + MAPWIDTH * 220 / (1000 - (y2)));

  thickLineColor (current.renderer, x1, y1, x2, y2, 2, palette[c]);
}

/* Possibly optimized horizontal line version of the above.
 * Not actually. */
void
HLine (int x1, int y1, int x2, int c)
{
  Line1 (x1, y1, x2, y1, c);
}

/* Do nothing. Guarranteed to be bug-free. */
void
WaitRetrace (void)
{
}

/* Construct a SDL_gfx friendly 32-bit pallette from 80bit cmap (see
 * cmap.c). */
void
SetPalette (Palette * pal)
{
  int             i = 0;
  do
    {
      palette[i] = 0xff000000;
      palette[i] |= (pal->color[i].red * 4) << 0;
      palette[i] |= (pal->color[i].green * 4) << 8;
      palette[i] |= (pal->color[i].blue * 4) << 16;
    }
  while (++i < COLORS);
}
