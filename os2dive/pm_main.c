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
 * Copyright(c)1996 by Thomas A. K. Kjaer                   *
 *                     See README for license details.      *
 *----------------------------------------------------------*
 * pm_main.c PM event handler for OS/2                      *
 ***********************************************************/
#define INCL_DOS
#define INCL_GPI
#define INCL_WIN

#include <os2.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define  _MEERROR_H_
#include <mmioos2.h>		/* It is from MMPM toolkit           */
#include <dive.h>
#include <fourcc.h>

/* min. includes */

#include "koules.h"
#include "pm_main.h"

extern void     close_device (void);

MRESULT EXPENTRY 
vgaAppWindowProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
  PWINDATA        _pwinData;	/* Pointer to window data               */
  SWP             swp;

  /* Get the pointer to window data. */
  if ((_pwinData = (PWINDATA) WinQueryWindowULong (hwnd, 0)))
    {
      switch (msg)
	{
	case WM_CREATE:
	  vga_wm_create (hwnd, _pwinData->hab);

	  return FALSE;

	  /* GameSrvr stuff */
	case WM_NotifyVideoModeChange:
	  if (((ULONG) mp1 == 1) && ((ULONG) mp2 == 0))
	    vga_os2_setFSDiveFullScreen (FALSE);

	  if (((ULONG) mp1 == 1) && ((ULONG) mp2 >= 2))
	    vga_os2_setFSDiveFullScreen (TRUE);

	  if ((ULONG) mp1 == 0)
	    _pwinData->fModeChange = TRUE;
	  else
	    _pwinData->fModeChange = FALSE;
	  break;

	case WM_INITMENU:
	  if (!vga_os2_presentFSDive ())
	    (VOID) WinEnableMenuItem (_pwinData->hwndMenu, IDM_FULLSCR, FALSE);
	  break;

	case WM_MOUSEMOVE:
	  WinQueryWindowPos (hwnd, &swp);
	  if (!vga_os2_FSDiveFullScreen ())
	    {
	      mousex = ((MOUSEMSG (&msg)->x) * _pwinData->ulWidth) / (swp.cx);
	      mousey = (MAPHEIGHT + 19) -
		((MOUSEMSG (&msg)->y) * _pwinData->ulHeight) / (swp.cy);
	      WinSetPointer (HWND_DESKTOP, NULLHANDLE);
	    }
	  else
	    {
	      /* This is really stupid!!
	         *
	         * When in Full-Screen Dive, the mouse resolution is
	         * 80 x 25!!!
	         * and your y-coord is 575 - 600 on x 800x600 display
	       */
	      mousex = ((MOUSEMSG (&msg)->x) * _pwinData->ulWidth) / 80;
	      mousey = ((RealScreenY - 1 - MOUSEMSG (&msg)->y)
			* _pwinData->ulHeight) / 25;
	    }
	  return MRFROMSHORT (1);

	case WM_BUTTON1DOWN:
	case WM_BUTTON2DOWN:
	  mousebuttons = 1;

	  break;

	case WM_BUTTON1UP:
	case WM_BUTTON2UP:
	  mousebuttons = 0;

	  break;

	case WM_COMMAND:
	  switch (SHORT1FROMMP (mp1))
	    {
	    case IDM_DESKSCR:
	      if (vga_os2_presentFSDive () && vga_os2_FSDiveFullScreen ())
		WinSendMsg (_pwinData->hwndFrame,
			    WM_SetVideoMode,
			    (MPARAM) WS_DesktopDive, 0);
	      break;

	    case IDM_FULLSCR:	/* GameSrvr */
	      if (vga_os2_presentFSDive ())
		WinSendMsg (_pwinData->hwndFrame,
			    WM_SetVideoMode,
			    (MPARAM) WS_FullScreenDive, 0);
	      break;

	    case IDM_SNAP:
	      {
		/* Find the total width and height of the window such that
		   * the actual video area equals the source width and height.
		 */
		ULONG           ulHeight = _pwinData->ulHeight;
		ULONG           ulWidth = _pwinData->ulWidth;
		ulHeight +=
		  WinQuerySysValue (HWND_DESKTOP, SV_CYSIZEBORDER) * 2;
		ulHeight +=
		  WinQuerySysValue (HWND_DESKTOP, SV_CYBORDER) * 2;
		ulHeight +=
		  WinQuerySysValue (HWND_DESKTOP, SV_CYMENU);
		ulHeight +=
		  WinQuerySysValue (HWND_DESKTOP, SV_CYTITLEBAR);
		ulWidth +=
		  WinQuerySysValue (HWND_DESKTOP, SV_CXSIZEBORDER) * 2;

		/* Set the new size of the window, but don't move
		   * the lower-left corner. 
		 */
		WinSetWindowPos (_pwinData->hwndFrame, NULLHANDLE,
				 100, 100, ulWidth, ulHeight,
				 SWP_SIZE | SWP_ACTIVATE);
	      }
	      break;

	    case VGA_ID_NEWTEXT:
	      /* Write new text string to the title bar */
	      WinSetWindowText (_pwinData->hwndFrame, (PSZ) mp2);
	      break;

	    case IDM_CENTER:	/* Center windows */
	      {
		ULONG           WinPosX, WinPosY;

		WinPosX = (RealScreenX / 2) - (GetSnapWidth (_pwinData->ulWidth) / 2);
		WinPosY = (RealScreenY / 2) - (GetSnapHeight (_pwinData->ulHeight) / 2);

		WinSetWindowPos (_pwinData->hwndFrame,
				 HWND_TOP,
				 WinPosX,
				 WinPosY,
				 0,
				 0,
				 SWP_MOVE | SWP_ACTIVATE | SWP_SHOW);
	      }
	      break;

	    case IDM_EXIT:

	      /* Post to quit the dispatch message loop. */
	      WinPostMsg (hwnd, WM_QUIT, 0L, 0L);
	      break;


	    default:
	      /* Let PM handle this message. */
	      return WinDefWindowProc (hwnd, msg, mp1, mp2);
	    }
	  break;

	case WM_VRNDISABLED:

	  DiveSetupBlitter (_pwinData->hDive, 0);

	  break;

	case WM_VRNENABLED:

	  vga_wm_vrnenabled (hwnd);

	  break;

	case WM_CHAR:
	  /* Character input: first two byte of message is the character code. */
	  if (SHORT2FROMMP (mp2) == VK_F4)
	    pp = 0;
	  else if (SHORT2FROMMP (mp2) == VK_F10)
	    WinPostMsg (hwnd, WM_COMMAND, (PVOID) IDM_DESKSCR, 0L);

	  /* Check for KeyRelease */
	  if (CHARMSG (&msg)->fs & KC_KEYUP)
	    {
	      deletekey (CHARMSG (&msg)->scancode);
	      /* check for some VK_keys */
	      switch (SHORT2FROMMP (mp2))
		{
		case VK_ESC:
		  pesc = 0;
		  break;
		case VK_UP:
		  pup = 0;
		  break;
		case VK_DOWN:
		  pdown = 0;
		  break;
		case VK_LEFT:
		  pleft = 0;
		  break;
		case VK_RIGHT:
		  pright = 0;
		  break;
		case VK_ENTER:
		case VK_NEWLINE:
		  penter = 0;
		  break;
		}

	      /* test for 'p' */
	      if (CHARMSG (&msg)->fs & KC_CHAR)
		{
		  switch (CHARMSG (&msg)->chr)
		    {
		    case 'o':
		    case 'O':
		      DosBeep (100, 300);
		      pp = 0;
		      break;
		    }
		}
	    }
	  /* Check for KeyPress */
	  else
	    {
	      if (((CHARMSG (&msg)->fs & KC_KEYUP) == 0) &&
		  ((CHARMSG (&msg)->fs & KC_PREVDOWN) == 0))
		{
		  addkey (CHARMSG (&msg)->scancode);
		  /* check for some VK_keys */
		  switch (SHORT2FROMMP (mp2))
		    {
		    case VK_ESC:
		      pesc = 1;
		      break;
		    case VK_UP:
		      pup = 1;
		      break;
		    case VK_DOWN:
		      pdown = 1;
		      break;
		    case VK_LEFT:
		      pleft = 1;
		      break;
		    case VK_RIGHT:
		      pright = 1;
		      break;
		    case VK_ENTER:
		    case VK_NEWLINE:
		      penter = 1;
		      break;
		    }

		  /* test for 'p' */
		  if (CHARMSG (&msg)->fs & KC_CHAR)
		    {
		      switch (CHARMSG (&msg)->chr)
			{
			case 'p':
			case 'P':
			  pp = 1;
			  break;
			}
		    }
		}
	    }

	  break;

	case WM_REALIZEPALETTE:

	  vga_wm_realizepalette (hwnd);

	  break;

	case WM_DESTROY:
#ifdef SOUND
	  if (sndinit)
	    close_device ();
#endif
	  vga_wm_close ();

	  break;

	default:
	  /* Let PM handle this message. */
	  return WinDefWindowProc (hwnd, msg, mp1, mp2);
	}
    }
  else
    /* Let PM handle this message. */
    return WinDefWindowProc (hwnd, msg, mp1, mp2);

  return (FALSE);
}
