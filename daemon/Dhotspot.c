#ifdef DEBUG
#include <stdio.h>
#endif
#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern PLAYER ships[];
extern PLANET *planets;
extern short **map;
extern struct stats stats;
extern short usedship;
extern short npl;

/*
 * See if enemies are in range.
 * Return 0 if no problems, 1 if enemy ships can see you, and
 * 2 if enemy ports can pound you.
 *
 * If neutral planet can see you, give it a reasonable chance
 * (ZAPPROB/100) of firing at you unless flag is 0 (in which
 * case we are just trying to find an initial square).
 * Enemy ports have excellent chance of firing at you
 * (EZAPPROB/100).
 */
hotspot(pl, flag)
register PLAYER *pl;
short flag;
{
	register short i;
	short r, c;
	short dist;
	int ret;
	int zapprob;
	PLANET *pt;
	PLAYER *tpl;

	ret = 0;
	if (pl->flags & DIED)
		return(ret);	/* don't bother */

	/* give all planets a shot at pl */
	for (i = 0; i < stats.nport; i++) {
		pt = &planets[i];
		if ((pt->flags & ALIMASK) != (pl->flags & ALIMASK)) {
			r = abs(pt->rpos - pl->rpos);
			c = abs(pt->cpos - pl->cpos);
			dist = MAX(r, c);
			if (dist <= 2 || dist <= pt->builds) {
				ret = 2;
				if (!flag)
					continue;
				if (pt->flags & ALIMASK)	/* enemy */
					zapprob = EZAPPROB;
				else
					zapprob = ZAPPROB;	/* neutral */
				if (rndrange(0, 99) < zapprob) {
#ifdef DEBUG
(void)printf("port #%d at %d,%d (flags=%o octal) potshot at %s\n",
	(pt-planets), pt->rpos, pt->cpos, pt->flags, pl->ship->name);
(void)fflush(stdout);
#endif
					if (potshot(pt, dist, pl) < 0)
						return(-1);
				}
			}
		}
	}

	/*
	 * If there's no base in range, we have to scan for enemy
	 * ships to see if we should be on condition RED.
	 */
	if (ret == 0)
		for (i = usedship; i != -1; i = tpl->list) {
			tpl = &ships[i];
			if ((tpl->flags & ACTIVE) == 0)
				/* inactive ship */
				continue;

			if ((tpl->flags & ALIMASK) != (pl->flags & ALIMASK)) {
				/* enemy ship -- is it in range? */
				r = abs(pl->rpos - tpl->rpos);
				c = abs(pl->cpos - tpl->cpos);
				dist = MAX(r, c);
				if (dist <= PRANGE) {
					ret = 1;
					break;
				}
			}
		}

	/* condition should be set by the caller (doscan()) */
	return(ret);
}
