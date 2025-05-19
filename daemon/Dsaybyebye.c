#include "config.h"
#include "u.h"

extern short **map;

void needscan();
void lockscore();
void unlockscore();

/*
 * A player died somehow by natural causes (i.e., stupid playing :-) ).
 * Send him final statistics and message notifying him of death.
 * Assume that calling process called pmsg to explain cause of death.
 * killpl() will take care of decrementing statistics later.
 */
saybyebye(pl)
PLAYER *pl;
{
#ifdef DEBUG
(void)printf("entered saybyebye\n");
#endif
	if (pl->flags & ACTIVE) {
		if (pl->rpos != -1) {	/* prevent extra work */
			map[pl->rpos][pl->cpos] = EMPTY;
			needscan(pl->rpos, pl->cpos);
			/* this player doesn't get new scan */
			pl->flags &= ~DOSCAN;
			pl->rpos = -1;
		}
	}
	if ((pl->flags & DIED) == 0) {
		if (dopoint(pl, 0) < 0)
			return(-1);
		if (pmsg(UmPDIED, pl, "", -1) < 0)
			return(-1);
#ifdef DEBUG
(void)printf("saybyebye: calling lockscore\n");
#endif
		lockscore();
		(void)updatescore(pl);
		unlockscore();
		pl->flags |= DIED;
	}
	return(1);
}
