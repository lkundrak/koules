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
* Copyright(c)1995 by Jan Hubicka.See README for license   *
*                          details.                        *
*----------------------------------------------------------*
*  input.c input functions for OS/2                        *
***********************************************************/
#include "koules.h"
#undef next

/* and something for the keyboard handling */
BYTE penter, pup, pdown, pp, pesc, pleft, pright;
ULONG mousex=0, mousey=0, mousebuttons=0;

struct keyrec *findkey(CONST int sc)
{
    struct keyrec  *kkey = firstkey;

    while (kkey != NULL && kkey->scancode != sc)
      kkey = kkey->next;
    return (kkey);
}

void addkey (CONST int scancode)
{
    struct keyrec  *key;

    if (!findkey (scancode))
      {
	  key = malloc (sizeof (struct keyrec));
	  key->last = NULL;
	  key->next = firstkey;
	  if (firstkey != NULL)
	    firstkey->last = key;
	  key->scancode = scancode;
	  firstkey = key;
      }
}

void deletekey (CONST int scancode)
{
    struct keyrec  *key;
    if (!(key = findkey (scancode)))
      {
	  printf ("Key not found!\n");
	  return;
      }
    if (key->last)
      key->last->next = key->next;
    else
      firstkey = key->next;
    if (key->next)
      key->next->last = key->last;
    free (key);
}

int GetKey()
{
    if (firstkey != NULL	/* && firstkey->next != NULL */ )
      return (firstkey->scancode);
    return (0);
}

