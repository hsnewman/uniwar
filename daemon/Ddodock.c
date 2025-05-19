#include <stdio.h>
#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern PLANET *planets;
extern short **map;
extern int Mrows, Mcols;
extern int baudincr;

/*
 * dodock -- if player is next to a friendly base, dock.
 * If it's a friendly planet, we don't get quite as much juice.
 */
dodock()
{
	short r, c;
	short lastr, lastc;
	register short i, j;
	register short bxtra;
	PLANET *pp;

	r = CurrPl->rpos + 1;
	c = CurrPl->cpos - 1;
	if ((lastr = r - 2) < 1)
		lastr = 1;
	if ((lastc = c + 2) > Mcols)
		lastc = Mcols;
	if (r > Mrows)
		r = Mrows;
	if (c < 1)
		c = 1;

	/* add up the dock benefit */
	bxtra = 0;
	for (i = r; i >= lastr; i--) {
		for (j = c; j <= lastc; j++)
			if (OBJTYPE(map[i][j]) == PORT) {
				pp = &planets[OBJINDX(map[i][j])];
				if ((pp->flags & ALIMASK) !=
						(CurrPl->flags & ALIMASK))
					continue;
				else if (pp->builds == 5)
					bxtra += 2;
				else
					bxtra++;
			}
	}
	if (bxtra && dockpl(bxtra) == -1)
		return(-1);

	if (bxtra) {
		if (pmsg(UmTEXT, CurrPl, "Docked\n", -1) < 0)
			return(-1);
		if (psleep(CurrPl, PDOCK + baudincr) < 0)
			return(-1);
	} else if (pmsg(UmTEXT, CurrPl, "Not adjacent to base!\n", -1) < 0)
		return(-1);
	return(1);
}

/*
 * dockpl -- give CurrPl benefits of docking.  One becomes
 * "undocked" by moving with warp or impulse, or by being displaced
 * by a nova or torpedo.
 *
 * bxtra is determined by how many and what kind of ports the ship
 * is next to.
 */
dockpl(bxtra)
register short bxtra;
{
	register short dxtra;	/* if already docked, devs get extra benefit */

	if (CurrPl->flags & DOCKED)
		dxtra = 2;
	else {
		dxtra = 1;
		CurrPl->flags |= DOCKED;
	}

	if (CurrPl->energy < MAXNRGY) {
		CurrPl->energy += INCNRGY * bxtra;
		CurrPl->energy = MIN(CurrPl->energy, MAXNRGY);
	}
	if (CurrPl->shields < MAXSHIE) {
		CurrPl->shields += INCSHIE * bxtra;
		CurrPl->shields = MIN(CurrPl->shields, MAXSHIE);
	}
	if (CurrPl->torps < MAXTORP) {
		CurrPl->torps += INCTORP * bxtra;
		CurrPl->torps = MIN(CurrPl->torps, MAXTORP);
	}
	CurrPl->lifersrv = MAXLIFE;
	if (CurrPl->damage > 0) {
		CurrPl->damage -= INCDMG * bxtra * dxtra;
		CurrPl->damage = MAX(CurrPl->damage, 0);
	}
	/* need to look at destroyed devices */
	return(1);
}
