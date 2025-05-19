#include <stdio.h>
#include "config.h"
#include "u.h"

extern short **map;
extern int Mrows, Mcols;
extern PLAYER ships[];
extern PLANET *planets;
extern PLAYER *CurrPl;

void scoreit(), needscan();

/*
 * nova - there was a nova at (pt->rpos, pt->cpos).
 * Make two passes peeking at neighboring sectors.
 * First pass:
 *	Hit any object except stars with a 750-850 hit
 *	and displace them away if possible.
 * Second pass:
 *	If stars found, 90% chance that they will blow up.
 *	If they do, call nova() recursively.
 */
nova(pt)
PLANET *pt;	/* actually nova in disguise */
{
	PLANET newnova;
	PLANET *pt2;
	PLAYER *pl1, *pl2;
	short r, c;
	short br, bc;
	short sr, sc;
	short rmax, rmin, cmin, cmax;
	short type;
	short count;
	short obx;
	long hit;
	int ret;
	int what;

	/* tell the world */
	pl1 = (PLAYER *)NULL;
	pl2 = (PLAYER *)NULL;
	pt2 = (PLANET *)NULL;
	scoreit(0L, SDESSTR, 2, 0, CurrPl, pl2, pt);	/* score */
	if (hitpmsg(NDIED, 0, 0, pl1, pl2, pt, pt2) < 0)
		return(-1);
	map[pt->rpos][pt->cpos] = EMPTY;
	needscan(pt->rpos, pt->cpos);
	rmax = MIN(Mrows, pt->rpos+1);
	rmin = MAX(1, pt->rpos-1);
	cmin = MAX(1, pt->cpos-1);
	cmax = MIN(Mcols, pt->cpos+1);

	/* first pass -- look for non-stars */
	count = 0;
	for (r = rmax; r >= rmin; r--)
		for (c = cmin; c <= cmax; c++) {
			what = NOVA;
			if ((type = OBJTYPE(map[r][c])) == EMPTY)
				continue;

			if (type == STAR)
				count++;
			if (type != SHIP && type != PORT)
				continue;

			obx = OBJINDX(map[r][c]);
			hit = rndrange(75000, 85000);

			/* determine displacement of object */
			ret = direct(r, c, r - pt->rpos, c - pt->cpos
				, &br, &bc, &sr, &sc, 1, 0);
			if (ret == 0) {	/* was displaced */
				what |= DISPLAC;
				map[br][bc] = map[r][c];
				needscan(br, bc);
				map[r][c] = EMPTY;
				needscan(r, c);
			}
			if (type == SHIP) {
				pl2 = &ships[obx];
				if (what & DISPLAC) {
					pl2->rpos = br;
					pl2->cpos = bc;
				}
				if (dlvrhit(what, pl1, pl2, pt, pt2, hit, 3)
								< 0)
					return(-1);
			} else if (type == PORT) {
				pt2 = &planets[obx];
				if (what & DISPLAC) {
					pt2->rpos = br;
					pt2->cpos = bc;
				}

				/* game could end */
				if ((ret = dlvrhit(what, pl1, pl2, pt
					, pt2, hit, 4)) <= 0)
					return(ret);
			}
		}

	/* second pass -- look for stars */
	if (count == 0)
		return(1);	/* no stars */

	for (r = rmax; r >= rmin && count; r--)
		for (c = cmin; c <= cmax; c++) {
			if ((type = OBJTYPE(map[r][c])) != STAR)
				continue;
			count--;
			if (rndrange(0,99) < 90) {
				newnova.rpos = r;
				newnova.cpos = c;
				if ((ret = nova(&newnova)) <= 0)
					return(ret);
			}
		}

	return(1);
}
