#include "config.h"
#include "u.h"

extern PLAYER *CurrPl, ships[];
extern PLANET *planets;
extern int Uerror;
extern short **map;
extern int Mrows, Mcols;

extern void error();

/*
 * dophaser -- CurrPl wants to fire his phasers.
 */
dophaser()
{
	short r, c;
	short br, bc;
	short mapspot, obx;
	long nrgy;
	int flag;
	PLAYER *pl;
	STARSHIP *ship;
	PLANET *pt;

	/* convert to relative coordinates */
	nrgy = 0;
	if (abs_rel(CurrPl, &r, &c, &ship, &nrgy) < 0) {
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

	if (MAX(abs(r), abs(c)) > PRANGE) {
		Uerror = UETOOFAR;
		error("");
		return(1);
	}
	if (nrgy == -1 || nrgy == 0)
		nrgy = DEFPHAS;
	else
		nrgy *= 100;
	if (nrgy > MAXPHAS) {
		Uerror = UEQUAN;
		error("");
		return(1);
	}

	/* find out what's out there */
	mapspot = map[CurrPl->rpos + r][CurrPl->cpos + c];
	obx = OBJINDX(mapspot);

	switch(OBJTYPE(mapspot)) {
	case EMPTY:
	case STAR:	/* can't phaser stars and empty space */
		Uerror = UENOTARG;
		error("");
		return(1);

	case PORT:
		pt = &planets[obx];
		if ((pt->flags & ALIMASK) == (CurrPl->flags & ALIMASK)) {
			Uerror = UEHITFR;
			error("");
			return(1);
		}
		flag = 2;	/* CurrPl hits port */
		break;

	case SHIP:
		pl = &ships[obx];
		if ((pl->ship->flags & ALIMASK)==(CurrPl->flags & ALIMASK)) {
			Uerror = UEHITFR;
			error("");
			return(1);
		}
		flag = 1;	/* CurrPl hits pl */
		break;

	default:
		return(1);	/* internal error, but ignore it! */
	}
	return(phaser(CurrPl, pl, pt, nrgy, MAX(abs(r), abs(c)), flag));
}
