#include "config.h"
#include "u.h"

extern struct devinfo devinfo[];

/*
 * adjcoords -- user fired a torpedo or moved.  Damage to
 * torpedo/warp_engines and/or computer causes errors in
 * the coordinates.  Probability of error depends on the
 * extent of the damage as follows:
 *
 * 1) If device is damaged x units out of maxdmg, then
 *	the probability is x/(2*maxdmg) that error will occur.
 * 2) If the computer is damaged y units out of maxdmg, then
 *	y/(2*maxdmg) is added to the above probability.
 * 3) Close range coords ought to foul up less often than long
 *	range coords, so we divide the calculated probability
 *	by TRANGE - range.
 *
 * If probability was greater than 1/2, there is a 1/4 chance
 * of error being plus or minus 2 in any direction.  Otherwise,
 * error is no worse than 1 in any direction.
 *
 * Finally, don't let dev do better than it's best.
 */
void
adjcoords(pl, dev, dam, dr, dc)
PLAYER *pl;
int dev;
long dam;
register short *dr, *dc;
{
	register int prob;
	int err, subt, best;
	short dist;

	prob = (100*dam)/(2*devinfo[dev].maxdmg);
	if (damaged(DCOMPU, pl, &dam))	/* critically damaged */
		dam = devinfo[DCOMPU].maxdmg;
	prob += (100*dam)/(2*devinfo[DCOMPU].maxdmg);
	dist = MAX(abs(*dr), abs(*dc));
	if (TRANGE > dist)
		prob /= (TRANGE-dist);

	if (prob == 0 || rndrange(1, 100) > prob)
		return;	/* no adjustment */
	if (prob > 50 && (rnd() % 4)) {
		err = 4;	/* plus or minus 2 */
		subt = 2;
	} else {
		err = 2;	/* plus or minus 1 */
		subt = 1;
	}
	*dr += (rndrange(0, err) - subt);
	*dc += (rndrange(0, err) - subt);
	if (dev == DWARP)
		best = MRANGE;
	else
		best = TRANGE;
	if (abs(*dr) > best)
		*dr = (*dr < 0 ? -best : best);
	if (abs(*dc) > best)
		*dc = (*dc < 0 ? -best : best);
}
