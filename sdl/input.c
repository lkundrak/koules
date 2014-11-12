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
*  sdl/input.c input routines using SDL                    *
***********************************************************/

#include <interface.h>
#include <stdbool.h>
#include <stdio.h>

#include <SDL.h>

#define MAX_PRESSED 256
int             pressed[MAX_PRESSED];
int             n_pressed = 0;
int             last_pressed;

void
UpdateInput (void)
{
  SDL_Event       event;
  int             val = -1;

  if (!SDL_PollEvent (&event))
    return;

  switch (event.type)
    {
    case SDL_KEYDOWN:
      val = 1;
      last_pressed = event.key.keysym.sym;
      break;
    case SDL_KEYUP:
      val = 0;
      last_pressed = 0;
      break;
    case SDL_MOUSEBUTTONDOWN:
      n_pressed++;
      break;
    case SDL_MOUSEBUTTONUP:
      n_pressed--;
      break;
    case SDL_QUIT:
      uninitialize ();
      exit (0);
    case SDL_WINDOWEVENT:
      if (event.window.event == SDL_WINDOWEVENT_CLOSE)
        {
          uninitialize ();
          exit (0);
        }
    default:
      return;
    }

  if (val >= 0)
    {
      int i;

      for (i = 0; i < MAX_PRESSED; i++)
        {
          if (pressed[i] == event.key.keysym.sym)
            {
              n_pressed -= 1;
              pressed[i] = 0;
              break;
            }
        }

      if (val)
        {
          if (n_pressed < MAX_PRESSED - 1)
            {
              for (i = 0; i < MAX_PRESSED; i++)
                {
                  if (pressed[i] == 0)
                    {
                      n_pressed += 1;
                      pressed[i] = event.key.keysym.sym;
                    }
                  if (pressed[i] == event.key.keysym.sym)
                    break;
                }
            }
        }
    }
}

int
GetKey (void)
{
  int             key = last_pressed;

  last_pressed = 0;
  return key;
}

bool
Pressed (void)
{
  return n_pressed != 0;
}

bool
IsPressed (int key)
{
  int i;

  for (i = 0; i < MAX_PRESSED; i++)
    {
      if (pressed[i] == key)
        return 1;
    }
  return 0;
}

int
IsPressedUp (void)
{
  return IsPressed(SDLK_UP);
}

int
IsPressedDown (void)
{
  return IsPressed(SDLK_DOWN);
}

int
IsPressedLeft (void)
{
  return IsPressed(SDLK_LEFT);
}

int
IsPressedRight (void)
{
  return IsPressed(SDLK_RIGHT);
}

int
IsPressedEnter (void)
{
  return IsPressed(SDLK_RETURN) || IsPressed(SDLK_KP_ENTER);
}

int
IsPressedEsc (void)
{
  return IsPressed(SDLK_ESCAPE);
}

int
IsPressedH (void)
{
  return IsPressed(SDLK_h);
}

int
IsPressedP (void)
{
  return IsPressed(SDLK_p);
}

#ifdef MOUSE
int
MouseX (void)
{
  int             x;
  SDL_GetMouseState (&x, NULL);
  return x;
}

int
MouseY (void)
{
  int             y;
  SDL_GetMouseState (NULL, &y);
  return y;
}

int
MouseButtons (void)
{
  return SDL_GetMouseState (NULL, NULL);
}
#endif
