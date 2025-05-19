#include "config.h"
#include "u.h"

extern PLAYER *CurrPl, ships[];
extern PLANET *planets;
extern int Uerror;
extern short **map;
extern int Mrows, Mcols;

extern void error();

/*
 * dotorp -- CurrPl wants to fire his torpedoes.
 * We allow up to a burst of three, and they all have
 * to go to same target.
 */
dotorp()
{
	short r, c;
	short br, bc;
	short mapspot, obx;
	long burst;
	PLAYER *pl;
	STARSHIP *ship;
	PLANET *pt;

	/* convert to relative coordinates */
	burst = 0;
	if (abs_rel(CurrPl, &r, &c, &ship, &burst) < 0) {
		/* print ship name expecting error msg to follow */
		if (Uerror == UEISEMPTY || Uerror == UEISDEAD) {
			error(ship->name);
		} else
			error("args invalid");
		return(1);	/* game continues */
	}

	br = CurrPl->rpos + r;
	bc = CurrPl->cpos + c;
	if (br < 1 || br > Mrows || bc < 1 || bc > Mcols) {
		Uerror = UEOUTGAL;
		error("");
		return(1);
	}

	if (MAX(abs(r), abs(c)) > TRANGE) {
		Uerror = UETOOFAR;
		error("");
		return(1);
	}
	if (burst == -1 || burst == 0)
		burst = 1;
	if (burst > 3) {
		Uerror = UEQUAN;
		error("");
		return(1);
	}

	/*
	 * find out what's out there -- can't put friendly ship or
	 * port in your torp sights (also prevents shooting at yourself).
	 */
	mapspot = map[br][bc];
	obx = OBJINDX(mapspot);

	switch(OBJTYPE(mapspot)) {
	case PORT:
		pt = &planets[obx];
		if ((pt->flags & ALIMASK) == (CurrPl->flags & ALIMASK)) {
			Uerror = UEHITFR;
			error("");
			return(1);
		}
		break;

	case SHIP:
		pl = &ships[obx];
		if ((pl->ship->flags & ALIMASK)==(CurrPl->flags & ALIMASK)) {
			Uerror = UEHITFR;
			error("");
			return(1);
		}
		break;

	case EMPTY:
	case STAR:
	default:
		return(1);	/* internal error, but ignore it! */
	}

	return(torp(CurrPl, burst, r, c));
}
