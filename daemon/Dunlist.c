#include "config.h"
#include "u.h"

extern short usedship;
extern PLAYER ships[];
extern struct stats stats;
extern PLANET *planets;

/*
 * unlist -- turn off RADIO flag on either SHIPS, PORTS, or both.
 * The RADIO flag is used to mark an item as part of a list,
 * either for ships to TELL (talk to) or things to LIST.
 */
void
unlist(flags)
int flags;
{
	register int i;
	register PLANET *pt, *ptL;

	if (flags & KSHIPS)
		for (i = usedship; i != -1; i = ships[i].list)
			if (ships[i].flags & ACTIVE)
				ships[i].ship->flags &= ~RADIO;
	if (flags & KPORTS) {
		ptL = planets + stats.nport;
		for (pt = planets; pt < ptL; ++pt)
			pt->flags &= ~RADIO;
	}
}
