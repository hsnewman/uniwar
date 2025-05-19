#include <stdio.h>
#include "config.h"
#include "u.h"

/*
 * potshot -- give planet (or base) a shot at a pl.
 * The strength of the shot depends on the builds of the
 * planet.  Neutrals and 0-built planets fire 50 units,
 * planets with 1-5 builds fire (100*builds) per bolt.
 *
 * There is a 25% chance that an extra shot will be fired.
 */
potshot(pt, dist, pl)
PLANET *pt;
short dist;
PLAYER *pl;
{
	long nrgy;

	if (pt->builds < 1)
		nrgy = 5000;
	else
		nrgy = pt->builds * 10000;

	/* pt hits pl */
	if (phaser((PLAYER *)NULL, pl, pt, nrgy, dist, 3) < 0)
		return(-1);

	/* 25% chance of another shot! */
	if (!(rnd() % 4))
		return(phaser((PLAYER *)NULL, pl, pt, nrgy, dist, 3));
	return(1);
}
