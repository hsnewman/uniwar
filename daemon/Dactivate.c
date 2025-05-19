#include "config.h"
#include "u.h"

extern short **map;
extern PLAYER *CurrPl;
extern PLAYER ships[];
extern struct stats stats;
extern int Mrows, Mcols;

void needscan();

/*
 * Activate CurrPl.  The player already has an active ship assigned
 * and is flagged as ACTIVE.
 */
activate()
{
	PLAYER tpl;
	register short rpos, cpos;
	register short trycnt;

	/* look for a spot that isn't attacked by enemy port */
	for (trycnt = 0; trycnt < 50; trycnt++) {
		tpl.rpos = rpos = rndrange(1, Mrows);
		tpl.cpos = cpos = rndrange(1, Mcols);
		/* if after 25 tries we can't find a cool spot,
		 * bring player into a hotspot if possible.
		 */
		if (map[rpos][cpos] == EMPTY &&
				(trycnt > 25 || hotspot(&tpl, 0) < 2))
			break;
	}
	if (trycnt >= 50) {
		if (pmsg(UmPDIED, CurrPl, "", 0) < 0)
			return(-1);
		CurrPl->flags |= DIED;
		return(1);
	}
	/* player made it into the game */
	if (CurrPl->flags & EMPIRE) {
		stats.totEcnt++;
		stats.Ecnt++;
	} else {
		stats.totFcnt++;
		stats.Fcnt++;
	}
	CurrPl->rpos = rpos;
	CurrPl->cpos = cpos;
	map[rpos][cpos] = (short)(CurrPl-ships) | (SHIP << 12);
	needscan(rpos, cpos);	/* flag ships that need to scan */

	if (allscan() < 0)	/* error sending message to sombeody */
		return(-1);
	if ((CurrPl->flags & DIED) == 0)
		return(dostatus(CurrPl));
	return(1);	/* player died. */
}
