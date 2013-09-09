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
* newbuttons.c New animated button-class for OS/2 Koules   *
***********************************************************/
#define ID_TIMER 1

static HPOINTER	   hIcon[16];
static ULONG	   ulIconIdx = 0;
static ULONG	   ulIconMax = 2;


typedef struct
{
    BOOL fHaveCapture ;
    BOOL fHaveFocus ;
    BOOL fInsideRect ;
    BOOL fSpaceDown ;
}
NEWBTN ;

typedef NEWBTN *PNEWBTN ;

MRESULT EXPENTRY NewBtnWndProc (HWND, ULONG, MPARAM, MPARAM) ;

BOOL APIENTRY RegisterNewBtnClass (HAB hab)
{
    return WinRegisterClass (hab, "NewBtn", NewBtnWndProc,
			     CS_SIZEREDRAW, sizeof (PNEWBTN)) ;
}

MRESULT EXPENTRY NewBtnWndProc (HWND hwnd, ULONG msg, MPARAM mp1, MPARAM mp2)
{
    BOOL          fTestInsideRect ;
    HPS           hps ;
    POINTL        ptl ;
    NEWBTN        *pNewBtn ;
    RECTL         rcl ;

    pNewBtn = WinQueryWindowPtr (hwnd, 0) ;

    switch (msg)
      {
	case WM_CREATE:
	  pNewBtn = malloc (sizeof (NEWBTN)) ;

	  pNewBtn->fHaveCapture = FALSE ;
	  pNewBtn->fHaveFocus   = FALSE ;
	  pNewBtn->fInsideRect  = FALSE ;
	  pNewBtn->fSpaceDown   = FALSE ;

	  WinSetWindowPtr(hwnd, 0, pNewBtn) ;

	  WinStartTimer(hab, hwnd, ID_TIMER, 333);

	  hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
	  hIcon[0] = WinLoadPointer(HWND_DESKTOP, 0, ID_KOULES);
	  hIcon[1] = WinLoadPointer(HWND_DESKTOP, 0, ID_KOULES2);
	  WinEndPaint(hps);

	  return 0 ;

	case WM_SETWINDOWPARAMS:

	  return MRFROMSHORT (1) ;

	case WM_QUERYWINDOWPARAMS:

	  return MRFROMSHORT (1) ;

	case WM_BUTTON1DOWN:
	  WinSetFocus (HWND_DESKTOP, hwnd) ;
	  WinSetCapture (HWND_DESKTOP, hwnd) ;
	  pNewBtn->fHaveCapture = TRUE ;
	  pNewBtn->fInsideRect  = TRUE ;
	  WinInvalidateRect (hwnd, NULL, FALSE) ;
	  return 0 ;

	case WM_MOUSEMOVE:
	  if (!pNewBtn->fHaveCapture)
	    break ;

	  WinQueryWindowRect (hwnd, &rcl) ;
	  ptl.x = MOUSEMSG(&msg)->x ;
	  ptl.y = MOUSEMSG(&msg)->y ;

	  fTestInsideRect = WinPtInRect (WinQueryAnchorBlock (hwnd),
					 &rcl, &ptl) ;

	  hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
	  if (pNewBtn->fInsideRect != fTestInsideRect)
	    {
		pNewBtn->fInsideRect = fTestInsideRect ;
		WinInvalidateRect (hwnd, NULL, FALSE) ;
	    }
	  WinEndPaint(hps);

	  break ;

	case WM_BUTTON1UP:
	  if (!pNewBtn->fHaveCapture)
	    break ;

	  WinSetCapture (HWND_DESKTOP, NULLHANDLE) ;
	  pNewBtn->fHaveCapture = FALSE ;
	  pNewBtn->fInsideRect  = FALSE ;

	  WinQueryWindowRect (hwnd, &rcl) ;
	  ptl.x = MOUSEMSG(&msg)->x ;
	  ptl.y = MOUSEMSG(&msg)->y ;

	  if (WinPtInRect (WinQueryAnchorBlock (hwnd), &rcl, &ptl))
	    WinPostMsg (WinQueryWindow (hwnd, QW_OWNER),
			WM_COMMAND,
			MPFROMSHORT (WinQueryWindowUShort (hwnd, QWS_ID)),
			MPFROM2SHORT (CMDSRC_OTHER, TRUE)) ;

	  WinInvalidateRect (hwnd, NULL, FALSE) ;
	  return 0 ;

	case WM_ENABLE:
	  WinInvalidateRect (hwnd, NULL, FALSE) ;
	  return 0 ;

	case WM_SETFOCUS:
	  pNewBtn->fHaveFocus = SHORT1FROMMP (mp2) ;
	  WinInvalidateRect (hwnd, NULL, FALSE) ;
	  return 0 ;

	case WM_CHAR:
	  if (!(CHARMSG(&msg)->fs   &  KC_VIRTUALKEY) ||
	      CHARMSG(&msg)->vkey != VK_SPACE       ||
	      CHARMSG(&msg)->fs   &  KC_PREVDOWN)
	    break ;

	  if (!(CHARMSG(&msg)->fs & KC_KEYUP))
	    pNewBtn->fSpaceDown = TRUE ;
	  else
	    {
		pNewBtn->fSpaceDown = FALSE ;
		WinPostMsg (WinQueryWindow (hwnd, QW_OWNER),
			    WM_COMMAND,
			    MPFROMSHORT (WinQueryWindowUShort (hwnd, QWS_ID)),
			    MPFROM2SHORT (CMDSRC_OTHER, FALSE)) ;
	    }
	  WinInvalidateRect (hwnd, NULL, FALSE) ;
	  return 0 ;

	case WM_TIMER:
          WinInvalidateRect (hwnd, NULL, FALSE);
	  if (++ulIconIdx == ulIconMax) ulIconIdx = 0;

	  return 0;

	case WM_PAINT:

	  hps = WinBeginPaint(hwnd, NULLHANDLE, NULL);
	  if (pNewBtn->fInsideRect || pNewBtn->fSpaceDown)
/*	    WinDrawPointer(hps, 0, 0, hIcon[ulIconIdx], DP_INVERTED) */;
	  else
	    WinDrawPointer(hps, 0, 0, hIcon[ulIconIdx], DP_NORMAL);
	  WinEndPaint(hps);

	  return 0 ;

	case WM_DESTROY:
	  free (pNewBtn);
	  WinDestroyPointer(hIcon[0]);
	  WinDestroyPointer(hIcon[1]);
	  WinStopTimer(hab, hwnd, ID_TIMER);
	  WinEndPaint(hps);
	  return 0;
      }
    return WinDefWindowProc (hwnd, msg, mp1, mp2) ;
}
