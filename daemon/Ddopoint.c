#include "config.h"
#include "u.h"

extern struct stats stats;
extern PLAYER ships[];
extern STARSHIP fleet[];
extern short usedship;
extern long allscore[][NSCORE];
extern long allstardate[];
extern char Outbuf[];

void cpsht(), cplng();
void lockscore();
void unlockscore();

/*
 * dopoint -- give point summary to pl (presume pl is active)
 * if flag is set, game has ended and we have to send stats to
 * all players (pl is ships[usedship] in this case).
 *
 * encode UmPOINT, short ship index, then the long point totals.
 */
dopoint(pl, flag)
PLAYER *pl;
int flag;
{
	char *hdr;
	short msize;
	register int i, j;

	if (usedship == -1)
		return(1);	/* no players in game */

	/* start the header */
	hdr = Outbuf+sizeof(short);
	*hdr++ = UmPOINT;
	cpsht((short)(pl->ship - fleet), &hdr);

	for (i = 0; i < NSCORE; i++) {
		cplng(pl->score[i], &hdr);
		cplng(allscore[FEDDIE][i], &hdr);
		cplng(allscore[EMPIRE][i], &hdr);
	}

	/* now info about ships */
	cpsht(stats.totFcnt, &hdr);
	cpsht(stats.totEcnt, &hdr);

	/* stardates */
	cplng(pl->stardate, &hdr);
	cplng(allstardate[FEDDIE], &hdr);
	cplng(allstardate[EMPIRE], &hdr);

	msize = hdr - Outbuf - sizeof(short);
	if ((pl->flags & DIED) == 0 && (pl->flags & ACTIVE)) {
		/* if not dead and is active (not pregame) */
		if (sendcmd(pl, msize) < 0)
			return(-1);
	}
	if (flag == 0)	/* only one player gets points */
		return(1);

	if (pmsg(UmPDIED, pl, "", 0) < 0)
		return(-1);

	lockscore();	/* prepare to update scorefile for all players */

	(void)updatescore(pl);
	pl->flags |= DIED;	/* player not in game */

	/* for each player, fill in the correct longs and kill them. */
	/* first ship will be skipped (flagged DIED above) */
	for (i = usedship; i != -1; i = pl->list) {
		pl = &ships[i];
		if (pl->flags & DIED)
			continue;
		if (pl->flags & ACTIVE) {
			hdr = Outbuf + sizeof(short) + 1;
			cpsht((short)(pl->ship - fleet), &hdr);
			for (j = 0; j < NSCORE; j++) {
				cplng(pl->score[j], &hdr);
				/* get past alliance info */
				hdr += 2*sizeof(long);
			}
			hdr += 2*sizeof(short);	/* get past ship totals */
			cplng(pl->stardate, &hdr);
			Outbuf[sizeof(short)] = UmPOINT;
			if (sendcmd(pl, msize) < 0) {
				unlockscore();
				return(-1);
			}
		}
		if (pmsg(UmPDIED, pl, "", 0) < 0) {
			unlockscore();
			return(-1);
		}
		(void)updatescore(pl);
		pl->flags |= DIED;	/* player not in game */
	}
	unlockscore();
	return(1);
}
