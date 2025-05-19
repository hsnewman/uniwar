#include <stdio.h>
#include "config.h"
#include "u.h"

extern short **map;
extern int Mrows, Mcols;
extern short *Mapbuf;
extern PLAYER ships[];
extern short npl, npg;
extern short availship, usedship;
extern struct stats stats;
extern long allscore[][NSCORE];
extern long allstardate[];
extern PLANET *planets;
extern int Uerror;
extern int baudincr;
extern int nocleanup;	/* set if cleanup should not be done on interrupt */

char *calloc();

/*
 * Create the universe, put objects in it, etc.
 * Initialize player structures and list of available ships.
 */
universe(nstars, maxbases, nplanets)
int nstars;	/* number of stars */
int maxbases;	/* bases per side */
int nplanets;	/* number of planets */
{
	register i, r, c;
	int fails;
	int side;	/* whose bases are we creating? */
	PLANET *pp;
#ifdef DEBUG
	char *strside;
#endif

	if (maxbases == -1 || maxbases > MAXBASES)
		maxbases = MAXBASES;

	/* allocate the galaxy */
	if ((map = (short **)calloc((unsigned)(Mrows+1), sizeof(short *))) == NULL) {
		Uerror = UEUNIX;
		return(-1);
	}
	if ((Mapbuf = (short *)calloc((unsigned)((Mrows+1)*(Mcols+1))
				, sizeof(short))) == NULL) {
		Uerror = UEUNIX;
		return(-1);
	}
	for (r = 0; r <= Mrows; r++) {
		map[r] = Mapbuf;
		Mapbuf += Mcols+1;
	}

	for (r = 0; r <= Mrows; r++)
		for (c = 0; c <= Mcols; c++)
			map[r][c] = EMPTY;

	/*
	 * create the stars -- plus or minus 50% of NSTARS
	 * If we try to put a star on top of another star, we
	 * succeed (so actually there may be fewer than nstars stars).
	 */
	/* can't have more than 1/2 of galaxy full of stars */
	if (nstars == -1 || nstars > Mrows*Mcols/2)
		nstars = rndrange(NSTARS/2, 3*NSTARS/2);
	for (i = 0; i < nstars; i++) {
		r = rndrange(1, Mrows);
		c = rndrange(1, Mcols);
		map[r][c] = STAR<<12;
	}
	/*
	 * create the planets -- between MINPLANETS and
	 * MAXPLANETS of them.  If we fail to find an empty
	 * spot MAXFAILS times, we give up.
	 */
	/* can't have more than 1/10 galaxy full of planets */
	if (nplanets < 0 || nplanets > Mrows*Mcols/10)
		nplanets = rndrange(MINPLANETS, MAXPLANETS);
	nplanets += maxbases*2;	/* count bases in nplanets */

	stats.nport = nplanets;
	if ((planets = (PLANET *)calloc((unsigned)nplanets
				, sizeof(PLANET))) == NULL) {
		Uerror = UEUNIX;
		return(-1);
	}

	pp = planets;
	side = FEDDIE;
	fails = 0;
	for (i = 0; i < nplanets; i++) {
		r = rndrange(1, Mrows);
		c = rndrange(1, Mcols);
		if (map[r][c] != EMPTY) {
			fails++;
			if (fails >= MAXFAILS) {
				Uerror = UECROWD;
				return(-1);
			}
			i--;
			continue;
		}
		/* i is index into planets[] */
		map[r][c] = (PORT<<12) + i;
		pp->rpos = r;
		pp->cpos = c;
		pp->atime = 0;
		pp->flags = side;
#ifdef DEBUG
switch(side) {
case FEDDIE:
	strside = "Federation";
	break;
case EMPIRE:
	strside = "Empire";
	break;
default:
	strside = "Neutral";
	break;
}
(void)printf("%s port at %d,%d\n", strside, pp->rpos, pp->cpos);
(void)fflush(stdout);
#endif

		if (side != 0) {
			pp->builds = 5;		/* base */
			pp->shields = MAXBSHIE;

			/* tell the side about the port */
			if (side == FEDDIE)
				pp->flags |= FKNOWS;
			else
				pp->flags |= EKNOWS;
		} else
			pp->builds = -1;	/* neutral */
		if (i % maxbases == maxbases-1) {
			if (side == FEDDIE)
				side = EMPIRE;
			else
				side = 0;
		}
		pp++;
	}
	stats.nplan[0] = nplanets - maxbases*2;
	stats.nplan[EMPIRE] = stats.nplan[FEDDIE] = 0;
	stats.nbase[EMPIRE] = stats.nbase[FEDDIE] = maxbases;

	/* initialize player structs */
	availship = 0;	/* 1st available ship */
	usedship = -1;	/* no used ships */
	for (i = 0; i < MAXPL; i++) {
		ships[i].pid = 1;	/* player structure unused */
		ships[i].list = i+1;	/* linked list of unused ships */
	}
	ships[MAXPL-1].list = -1;	/* end of free list */

	/* initialize statistics */
	stats.totEcnt = stats.totFcnt = 0;
	stats.Ecnt = stats.Fcnt = 0;
	stats.baudcnt[BAUD300] = stats.baudcnt[BAUD1200] = 0;
	baudincr = 0;

	/* no points yet -- player points cleared at activate time. */
	for (i = 0; i < NSCORE; i++) {
		allscore[FEDDIE][i] = 0;
		allscore[EMPIRE][i] = 0;
	}
	allstardate[FEDDIE] = allstardate[EMPIRE] = 0;

	nocleanup = 0;	/* cleanup on interrupt */
	return(0);
}
