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
/* Changes: joystick A button included into buttons        *  
 *  for which function "Pressed()" returns 1               *
 *  (c) 1997 by Ludvik Tesar (Ludv\'{\i}k Tesa\v{r})       *
 ************************LT*********************************/
#include <vga.h>
#include <vgagl.h>
#include <vgakeyboard.h>
#include <vgamouse.h>
#ifdef JOYSTICK
extern int      joystickdevice[2];
#endif

#define SCANCODE_P 25

#define IsPressed(scancode) keyboard_keypressed(scancode)
#ifdef MOUSE
#define UpdateInput() keyboard_update (),(!nomouse?mouse_update ():0)
#define MouseButtons() (!nomouse?mouse_getbutton():0)
#define MouseX() (!nomouse?mouse_getx():0)
#define MouseY() (!nomouse?mouse_gety():0)
#else
#define UpdateInput() keyboard_update ()
#define MouseButtons() DUMMY
#define MouseX() DUMMY
#define MouseY() DUMMY
#endif

/*here is probably bug....
   some users reported that these functions not work...
   probably sometimes some keys are reported as pressed */
static inline int
Pressed ()
{
  int             z;
#ifdef JOYSTICK
  int             status;
  struct JS_DATA_TYPE js;
  if(joystickdevice[0]>=0)
     {
	status = read (joystickdevice[0], &js, JS_RETURN);
	if ((status == JS_RETURN)&&js.buttons)return 1;
     }
#endif
#ifdef MOUSE
  if (!nomouse && MouseButtons ())
    return (1);
#endif
  for (z = 0; z < 128; z++)
    if (keyboard_keypressed (z))
      {
	return 1;
      }
  return (0);
}
static inline int
GetKey ()
{
  int             z, k = 0;
#ifdef MOUSE
  if (!nomouse && MouseButtons ())
    return (1);
#endif
  for (z = 0; z < 128; z++)
    if (keyboard_keypressed (z))
      {
	if (k)
	  return (0);
	k = z;
      }
  return (k);
}
#define IsPressedEsc() IsPressed (SCANCODE_ESCAPE)

#define IsPressedP() IsPressed (SCANCODE_P)
#define IsPressedH() IsPressed (35)

#define IsPressedEnter() (IsPressed (SCANCODE_ENTER)||\
                        IsPressed (SCANCODE_SPACE)||\
                        IsPressed (SCANCODE_KEYPADENTER))

#define IsPressedUp() (IsPressed (SCANCODE_CURSORUP) ||\
		     IsPressed (SCANCODE_CURSORBLOCKUP))

#define IsPressedDown() (IsPressed (SCANCODE_CURSORDOWN) ||\
		     IsPressed (SCANCODE_CURSORBLOCKDOWN))
#define IsPressedLeft() (IsPressed (SCANCODE_CURSORLEFT) ||\
		     IsPressed (SCANCODE_CURSORBLOCKLEFT))

#define IsPressedRight() (IsPressed (SCANCODE_CURSORRIGHT) ||\
		     IsPressed (SCANCODE_CURSORBLOCKRIGHT))
