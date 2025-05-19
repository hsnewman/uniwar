#include <stdio.h>
#include "config.h"
#include "u.h"

extern struct devinfo devinfo[];
extern int baudincr;

long time();

/*
 * phaser -- fire a phaser blast.  Either pl1 hits pl2, pl1 hits pt,
 * or pt hits pl2 depending whether flag is 1, 2, or 3 respectively.
 * distance between combatants is dist.  nrgy is units of nrgy fired
 * times 100.  Calling routine should verify that we're not hitting
 * somebody that's on our side.
 * NOTE, torpedoes and novas hit things via another routine (the other
 * routine handles displacment).
 */
phaser(pl1, pl2, pt, nrgy, dist, flag)
PLAYER *pl1, *pl2;
PLANET *pt;
long nrgy;
short dist;
int flag;
{
	long *phtime;	/* time current phaser bank was last fired */
	long onrgy;	/* original energy */
	long hit, dam;
	long tmp;	/* temporaries */
	long minuspct;	/* pct of bolt lost due to device damage */
	short sfctr;	/* if shielded, costs energy to raise and lower shlds */
	long thistime;
	int sleeptime=0; /* players that fire phasers have to sleep */
	int ret;
	PLANET *pt1, *pt2;

	/*
	 * Determine phaser effectiveness.
	 * Make blast inversely proportional to distance from
	 * target (not square of distance because universe
	 * is two dimensional and decwar did it that way).
	 *
	 * We need a random factor, so we pick a random value
	 * plus or minus 20 percent of the calculated hit.
	 */

	/* first, make sure player has enough energy */
	minuspct = 0;
	switch(flag) {
	case 1:	/* pl1 hits pl2 */
	case 2:	/* pl1 hits pt */
		if (damaged(DPHASER, pl1, &dam)) /* 0 if device usable */
			return(critmsg(pl1, DPHASER));
		if (pl1->flags & SHIELDED)
			sfctr = 20000;
		else
			sfctr = 0;
		if (pl1->energy < sfctr + nrgy) {
			if (pmsg(UmTEXT, pl1, "Insufficient energy\n", -1) < 0)
				return(-1);
			return(1);
		}

		/*
		 * sleeptime depends on last time phasers were fired
		 * and also on damage.
		 */

		/* phasers needed this long to cool */
		sleeptime = PPHASER + dam/10000 + baudincr;

		/* fire phaser bank that was fired longest ago */
		if (pl1->phtime1 < pl1->phtime2)
			phtime = &pl1->phtime1;
		else
			phtime = &pl1->phtime2;

		/* phasers already cooled for (thistime - *phtime) seconds */
		thistime = time(0L);
		sleeptime -= (thistime - *phtime);
		if (sleeptime > 0) {
			/* must wait to do command */
			/* if 2 is returned, no sleep was needed */
			if ((ret = psleep(pl1, sleeptime)) != 2) {
				/* command must be reparsed after sleep */
				pl1->plbuf.start = pl1->plbuf.ostart;
				return(ret);
			}
		}
		/* can do command now */
		*phtime = thistime;

		if (sfctr > 0 && (pl1->flags & OSHORT) == 0) {
			if (pmsg(UmTEXT, pl1,
				"(Hi-speed shield ctrl activated)\n", -1) < 0)
				return(-1);
		}
		(void)decnrgy(pl1, sfctr + nrgy);
		onrgy = nrgy;

		/*
		 * hit diminished if damage to phasers and/or computer.
		 * Divide by 100 to help prevent overflow.
		 */
		/* damage to phasers?  Can lose up to 50% of bolt */
		minuspct = (dam/100)/(2*devinfo[DPHASER].maxdmg/100);

		/* damage to computers?  Can lose up to 50% more! */
		if (damaged(DCOMPU, pl1, &dam))
			dam = devinfo[DCOMPU].maxdmg;
		minuspct += (dam/100)/(2*devinfo[DCOMPU].maxdmg/100);

		/* see if the phasers got damaged */
		if (onrgy <= DEFPHAS)
			tmp = 5;	/* 5% chance of damage */
		else
			/* max 65% chance */
			tmp = (onrgy - DEFPHAS)/(MAXPHAS/500) + 5;
		if (rndrange(0, 99) < tmp) {
			/* phasers damaged! */
			pl1->damdev[DPHASER] += rndrange((int)onrgy/2
							, (int)nrgy);
			if ((pl1->flags & OSHORT) == 0) {
				if (pmsg(UmTEXT, pl1, "Scotty: \"$#*&!\"\n"
								, -1) < 0)
					return(-1);
			}
			if (pmsg(UmTEXT, pl1, "Phasers damaged!\n", -1) < 0)
				return(-1);
		}
		break;
	default:
		break;
	}

	/*
	hit = ((2 * nrgy)/5) * (PRANGE - dist + 1);
	*/
	hit = nrgy * ((3*PRANGE) - 2*(dist+1))/10;
	hit = rndrange((int)((4*hit)/5), (int)((6*hit)/5));
	hit = hit - (hit*minuspct)/100;
#ifdef DEBUG
printf("*** phaser hit size=%d, nrgy=%d, dist=%d\n", hit/100, nrgy/100, dist);
#endif

	/* game over if 0 returned, fatal error if -1 returned */
	if (flag == 2)	/* pt was victim */
		pt2 = pt;
	else
		pt1 = pt;
	if ((ret = dlvrhit(FASER, pl1, pl2, pt1, pt2, hit, flag)) <= 0)
		return(ret);

	return(1);
}
