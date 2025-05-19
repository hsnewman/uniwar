#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern struct devinfo devinfo[];

/*
 * repair -- repair CurrPl's devices by units.
 * Calling routine responsible for any waiting.
 * Return max amount actually repaired.
 */
repair(units)
register int units;
{
	register int dockflag;
	register int i;
	int tmp, max;

	if (CurrPl->flags & DOCKED)
		dockflag = 1;
	else
		dockflag = 0;

	max = 0;
	for (i = 0; i < NDEVS; i++) {
		if (CurrPl->damdev[i] == 0)
			continue;
		/* don't repair destroyed devices except when docked */
		if (dockflag == 1 || CurrPl->damdev[i] <= 3*devinfo[i].maxdmg) {
			tmp = MIN(units, CurrPl->damdev[i]);
			if (tmp > max)
				max = tmp;
			CurrPl->damdev[i] -= tmp;
		}
	}
	return(max);
}
