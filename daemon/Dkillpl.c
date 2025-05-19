#include <stdio.h>
#include "config.h"
#include "u.h"

extern short usedship, availship;
extern PLAYER ships[];
extern short npg, npl;
extern struct stats stats;

void newbaud();

/*
 * Kill a player off.  We assume the appropriate messages have been
 * sent to him and score file has been updated.
 */
killpl(pl)
register PLAYER *pl;
{
	register short preplx, plx;
	char ofifonam[18];

	/*
	 * read all the stale messages that may have been sent to
	 * this player.
	 */
#ifdef DEBUG
if (pl->flags & ACTIVE)
	(void)printf("killing off the %s\n", pl->ship->name);
else
	(void)printf("killing off pregame pl %d\n", pl - ships);
(void)fflush(stdout);
#endif
	/* clean up */
	(void)close(pl->ofd);
	(void)sprintf(ofifonam, "/tmp/UnIwAr%05d", pl->pid);
	(void)unlink(ofifonam);	/* if this fails, player probably already
					took care of it */

	npl--;
	if ((pl->flags & ACTIVE) == 0)
		npg--;
	else if (pl->flags & EMPIRE)
		stats.Ecnt--;
	else	/* FEDDIE */
		stats.Fcnt--;

	pl->pid = 1;		/* unused */
	pl->pending = 0;	/* no msgs pending */
	if (pl->flags & ACTIVE) {
		pl->ship->flags &= ~ACTIVE;
		pl->ship->owner = (PLAYER *)NULL;
		/* map cleared in saybyebye() */
	}

	/* cleanup old baud rate */
	if (pl->baudrate < 1200)
		stats.baudcnt[BAUD300]--;
	else if (pl->baudrate < 9600)
		stats.baudcnt[BAUD1200]--;
	pl->baudrate = 9600;

	/* find him in list of used ships */
	for (preplx = plx = usedship; plx != -1 && &ships[plx] != pl
				; plx = ships[plx].list)
		preplx = plx;
	if (plx == -1)
		return(1); /* already removed from usedship list somehow... */
	if (preplx == plx)	/* first in list */
		usedship = pl->list;
	else
		ships[preplx].list = pl->list;

	/* add to front of list of available ships */
	pl->list = availship;
	availship = plx;
#ifdef DEBUG
if (pl->flags & ACTIVE)
	(void)printf("the %s is dead\n", pl->ship->name);
else
	(void)printf("pregame pl %d is dead\n", pl - ships);
(void)fflush(stdout);
#endif
	pl->ship = (STARSHIP *)NULL;
	pl->phtime1 = pl->phtime2 = pl->totime = 0;
	pl->flags = 0;		/* player is very dead */
	return(1);
}
