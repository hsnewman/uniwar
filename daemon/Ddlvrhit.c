#include <stdio.h>
#include "config.h"
#include "u.h"

extern struct stats stats;
extern short **map;
extern PLANET *planets;
extern PLAYER *CurrPl;

void scoreit(), needscan();
long time();

/*
 * dlvrhit -- deliver the hit.  We distribute the hit as follows:
 * IF SHIELDS UP
 * 1) If the victim has shields up at x%, x%
 *    of the hit is absorbed into the shields (causing the
 *    shield energy to be decremented by tha amount absorbed).
 *    So that shields will eventually reach 0, an additional
 *    amount equal to 1/4 of the hit is deducted from the
 *    shield energy (this latter true only if shields up).
 * 2) The remainder of the hit is multiplied by a number
 *    between 1 and 1.5 to determine total damage.
 * 3) Ship damage is increased by the size of the remainder
 *    of the hit.  This remainder is decremented by 100
 *    (actually 10000) and distributed among the ship's
 *    devices through the hitdevs() subroutine.
 *
 * IF SHIELDS DOWN
 * same as if shields up at 0%.
 *
 * 'what' is what happened:
 *	FASER-- somebody hit by phaser.
 *	TORP  -- somebody hit by torpedo.
 *	NOVA  -- somebody hit by nova.	
 *	DISPLAC -- if victim was displaced.
 *
 * 'flag' indicates who did what.  Either pl1 hits pl2, pl1 hits pt2,
 * pt1 hits pl2, or pt1 (nova) hits pt2, depending whether flag is 1,
 * 2, 3, or 4 respectively.
 * 'hit' is the size of the hit.
 */
dlvrhit(what, pl1, pl2, pt1, pt2, hit, flag)
int what;
PLAYER *pl1, *pl2;
PLANET *pt1, *pt2;
long hit;
int flag;
{
	register long t1, t2;
	long thistime;
	long shld;
	char *bdmsg;

#ifdef DEBUG
printf("*** hit size=%d\n", hit/100);
#endif
	switch(flag) {
	case 1:	/* pl1 hits pl2 */
	case 3:	/* pt1 hits pl2 */
		if (pl2->flags & SHIELDED)
			shld = pl2->shields;
		else
			shld = 0;

		/* fix overflow */
		t1 = hit/100;
		t2 = hit % 100;
		t1 = (t1 * shld)/MAXSHIE;
		t2 = ((t2 * shld)/MAXSHIE) + t1*100;

		/* lose the amount absorbed */
		pl2->shields -= MIN(t2, pl2->shields);
		/* to ensure we run out eventually */
		if (pl2->flags & SHIELDED)
			pl2->shields -= MIN(hit/4, pl2->shields);
		hit -= t2;
		hit = rndrange((int)hit, (int)(3*hit)/2);
		scoreit(what, SDMGENE, flag, hit, pl1, pl2, pt1);

		/* now cause ship damage and see if it killed pl2 */
		pl2->damage += hit;
		if (pl2->damage > MAXDMG) {
			scoreit(what, SDESENE, flag, hit, pl1, pl2, pt1);
			if (hitpmsg(what|RIP, flag, hit, pl1, pl2
						, pt1, pt2) < 0)
				return(-1);
			if (saybyebye(pl2) < 0)
				return(-1);
			return(1);	/* pl2 bit the big one */
		} else if (hitpmsg(what, flag, hit, pl1, pl2, pt1, pt2) < 0)
			return(-1);

		/* well, pl2 survived.  Lay some device damage on him */
		hit -= 10000;	/* arbitrarily don't let first 100 units
				   of hit reach devices.*/

		/* tell player his new status (don't scan, takes too long.) */
		if (dostatus(pl2) < 0)
			return(-1);

		/*
		 * you can't sustain device damage as long as your shields
		 * are up and >= 50%
		 */
		if (hit <= 0 || (pl2->shields >= MAXSHIE/2 &&
				(pl2->flags & SHIELDED)))
			break;	/* survived with flying colors */

		if (hitdevs(hit, pl2) < 0)	/* hit pl2's devices */
			return(-1);
		break;

	case 2:	/* pl1 hits pt2 */
	case 4:	/* pt1 (nova) hits pt2 */
		/*
		 * If hit by phaser, can't hurt if planet has no builds.
		 * To remove builds from planets takes heavy fire-power
		 * and luck.  There is no chance unless the hit is at
		 * least 100 units.  The chance increases from 1 in 100
		 * to 1 in 2 as units of hit increase from 100 to 500.
		 * hits above 500 all have 50-50 chance of removing a build.
		 *
		 * If hit by torp, there is a 33% chance of removing a build
		 * from a planet.  If there are no builds, there is a 33%
		 * chance of destroying the planet.
		 * 
		 * If hit by nova, automatically remove 3 builds.  If this
		 * reduces planet to less than 0 builds, planet is destroyed.
		 *
		 * This routine does not handle stars getting hit.  If
		 * a star is getting torpedoed, torpedo() will have to
		 * detect that.  When a star blows up, nova() is called to
		 * handle zapping all surrounding space.  nova() calls this
		 * routine when a ship or port gets hit.
		 */
		if (pt2->builds != 5) {	/* not a base */
			switch(what & (FASER|TORP|NOVA)) {
			case FASER:
				if (pt2->builds <= 0) {
					/* no builds -- waste of energy */
					if (hitpmsg(what, flag, 0L, pl1, pl2
							, pt1, pt2) < 0)
						return(-1);
				}
				else if (pt2->builds < 5) {
					t1 = rndrange(10000, 90000);
					if (t1 < hit) {
						if (pt2->builds > 2)
							needscan(pt2->rpos,
								pt2->cpos);
						pt2->builds--;
					}
					if (hitpmsg(what, flag, 0L, pl1, pl2
							, pt1, pt2) < 0)
						return(-1);
				}
				break;

			case TORP:
				if ((rnd() % 3) == 0) {
					if (pt2->builds <= 0) {
						scoreit(what, SDESPL, flag
							, hit, pl1, pl2, pt2);
						what |= RIP;
					} else {
						if (pt2->builds > 2)
							needscan(pt2->rpos,
								pt2->cpos);
						pt2->builds--;
					}
				}
				if (hitpmsg(what, flag, 0L, pl1, pl2, pt1
								, pt2) < 0)
					return(-1);
				break;

			case NOVA:
				if (pt2->builds - 3 < 0) {
					scoreit(what, SDESPL, flag, hit
							, CurrPl, pl2, pt2);
					what |= RIP;
					what &= ~DISPLAC;
				} else {
					if (pt2->builds > 2)
						needscan(pt2->rpos,
							pt2->cpos);
					pt2->builds -= 3;
				}
				if (hitpmsg(what, flag, 0L, pl1, pl2, pt1
								, pt2) < 0)
					return(-1);
				break;

			default:
				break;
			}

			if (what & RIP) {	/* remove the planet */
				/* remove the dead planet */
				stats.nplan[pt2->flags & ALIMASK]--;
				map[pt2->rpos][pt2->cpos] = EMPTY;
				needscan(pt2->rpos, pt2->cpos);
				stats.nport--;

				/* game might end */
				if (stats.nbase[EMPIRE] + stats.nplan[EMPIRE]
					== stats.nport ||
					stats.nbase[FEDDIE] +
					stats.nplan[FEDDIE] == stats.nport)
					return(0);

				/* copy last planet struct to new hole */
				if (pt2 != &planets[stats.nport]) {
					*pt2 = planets[stats.nport];

					/* fix map */
					map[pt2->rpos][pt2->cpos] =
						(PORT<<12) + (pt2 - planets);
				}
			}
			break;	/* all done */
		}
		/*
		 * Try to destroy a base.  When base's shields drop
		 * below 25%, there is a chance in 20 that it will
		 * be destroyed by a hit.  When base's shields drop
		 * to between 10 and 0, there is a fifty-fifty chance
		 * that they will go down.  Then, a random number
		 * between 0 and 9 inclusive is compared to the
		 * shield percent -- if the shield percent is lower,
		 * the base is destroyed, else, the shields stay up.
		 *
		 * When points are implemented, give credit for
		 * destruction of base to CurrPl (not pl1 since
		 * base may have been destroyed by NOVA).
		 */
		/* tell friends about the attack */
		thistime = time(0L);
		if (thistime - pt2->atime > PAWAIT) {
			pt2->atime = thistime;
			if (hitpmsg(PATTK, flag, hit, (PLAYER *)NULL
				, (PLAYER *)NULL, pt2, (PLANET *)NULL) < 0)
				return(-1);
		}

		/* fix overflow */
		shld = pt2->shields;
		t1 = hit/100;
		t2 = hit % 100;
		t1 = (t1 * shld)/MAXBSHIE;
		t2 = ((t2 * shld)/MAXBSHIE) + t1*100;

		/* lose entire amount absorbed */
		pt2->shields -= MIN(t2, pt2->shields);
		/* to ensure we run out eventually */
		pt2->shields -= MIN(hit/4, pt2->shields);
#ifdef DEBUG
printf("\t%d of hit on base absorbed, shields now at %d\n", t2, pt2->shields);
#endif
		hit -= t2;
		hit = rndrange((int)hit, (int)(3*hit)/2);
		scoreit(what, SDMGBAS, flag, hit, CurrPl, pl2, pt2);

		t1 = 0;	/* base lives */
		/* drop below 25% and chance in 20 happens, base dies */
		if (pt2->shields < MAXBSHIE/4 && (rnd() % 20) == 0)
			t1 = 1;	/* base died */

		/*
		 * drop below 10%, chance in 2 of shields failing.
		 * Use shield pct to determine odds of base dying.
		 */
		else if (pt2->shields < MAXBSHIE/10 &&
				(pt2->shields == 0 || rnd() % 2)) {
			if ((CurrPl->flags & OSHORT) == 0) {
bdmsg = "Base shields collapsed!\nAttempt emergency shield regen!\n";
				if (pmsg(UmTEXT, CurrPl, bdmsg, -1) < 0)
					return(-1);
			}
			if (rndrange(0, 9) > (100*pt2->shields)/MAXBSHIE)
				t1 = 1;	/* base died */
			if ((CurrPl->flags & OSHORT) == 0) {
				if (t1)
					bdmsg = "Emergency regen failed!\n";
				else
					bdmsg = "Emergency regen succeeded!\n";
				if (pmsg(UmTEXT, CurrPl, bdmsg, -1) < 0)
					return(-1);
			}
		}
		if (t1) {
			/* base destroyed */
			if (hitpmsg(RIP|what, flag, hit, pl1, pl2
				, pt1, pt2) < 0)
				return(-1);
			/* last gasp to far-away teammates with radios */
			if (hitpmsg(PDIED, flag, hit, (PLAYER *)NULL
				, (PLAYER *)NULL, pt2, (PLANET *)NULL) < 0)
				return(-1);
			/* get rid of the base */
			/* put this code in another routine later */
			--stats.nbase[pt2->flags & ALIMASK];

			map[pt2->rpos][pt2->cpos] = EMPTY;
			needscan(pt2->rpos, pt2->cpos);
			stats.nport--;

			/* copy last planet struct to new hole */
			if (pt2 != &planets[stats.nport]) {
				*pt2 = planets[stats.nport];

				/* fix map */
				map[pt2->rpos][pt2->cpos] =
					(PORT<<12) + (pt2 - planets);
			}

			/* declare the winner in cleanup */
			if (stats.nbase[EMPIRE] + stats.nplan[EMPIRE]
				== stats.nport ||
				stats.nbase[FEDDIE] + stats.nplan[FEDDIE]
				== stats.nport)
				return(0);

		/* else just hit, not destroyed */
		} else if (hitpmsg(what, flag, hit, pl1, pl2
				, pt1, pt2) < 0)
			return(-1);
		break;

	default:
		break;
	}
	return(1);
}
