#include <stdio.h>
#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern STARSHIP fleet[];

/*
 * dousers -- display players in the game.
 */
dousers()
{
	STARSHIP *shipL, *sh;
	char buf[(MAXPL+2)*32];

	shipL = &fleet[MAXPL];
	(void)sprintf(buf
		, " Captain           Ship\n---------         ------\n");
	for (sh = fleet; sh < shipL; sh++) {
		if ((sh->flags & ACTIVE) == 0)
			continue;
		(void)sprintf(&buf[strlen(buf)], "%-16s  %s\n",
			sh->owner->name, sh->name);
	}
	return(pmsg(UmTEXT, CurrPl, buf, -1));
}
