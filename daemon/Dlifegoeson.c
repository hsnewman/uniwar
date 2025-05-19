#include <signal.h>
#include "config.h"
#include "u.h"

extern PLAYER ships[];
extern short usedship;
extern PLANET *planets;
extern struct stats stats;
extern short npl;

/*
 * lifegoeson -- increment system stardate every time we complete an
 * all_cmd() loop.  When stardate reaches certain milestones,
 * perform certain tasks.
 */
void
lifegoeson()
{
	static long stardate;
	register PLANET *pp;
	register int i;

	if (++stardate < 0)	/* rollover */
		stardate = 0;

	/* regen BPCTREGEN every npl*10 system stardates */
	if (npl > 0 && (stardate % (npl*10)) == 0)	/* regenerate bases */
		for (i = 0; i < stats.nport; i++) {
			pp = &planets[i];
			if (pp->builds == 5 && pp->shields < MAXBSHIE)
				pp->shields = MIN(MAXBSHIE,
						pp->shields + BPCTREGEN);
		}
}
