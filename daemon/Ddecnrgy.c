#include "config.h"
#include "u.h"

/*
 * Decrement a player's energy by 'nrgy'.  If player runs out,
 * transfer all his shield energy to ship energy.
 */
decnrgy(pl, nrgy)
PLAYER *pl;
long nrgy;
{
	pl->energy -= nrgy; 
	if (pl->energy < 0) {
		if (pmsg(UmTEXT, pl, "You ran out of energy!!\n", -1) < 0)
			return(-1);
		pl->energy += pl->shields;
		pl->shields = 0;
		/* pl->flags &= ~SHIELDED; */
		if (pl->energy > 0) {
			if (pmsg(UmTEXT, pl, "Emergency shield transfer!!\n"
							, -1) < 0)
				return(-1);
		} else
			return(saybyebye(pl));
	}
	return(1);
}
