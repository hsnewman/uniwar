#include <ctype.h>
#include "config.h"
#include "u.h"

extern int baudincr;
extern struct devinfo devinfo[];
extern PLAYER *CurrPl;
extern short **map;
extern int Uerror;

void error(), adjcoords(), needscan();

/*
 * Move CurrPl's ship.  Do some parsing: syntax of arguments
 * remaining in cmdp is:
 *
 *	[Absolute|Relative|Computed] <vpos> <hpos>
 *
 * We call direct() to handle trajectory.
 */
domove(tok)
int tok;
{
	short dr, dc;	/* distance row and col */
	short or, oc;	/* original row and col */
	short sr, sc;	/* stuck row and col (if necessary) */
	short br, bc;	/* destination (or blocked) row and col */
	STARSHIP *ship;
	short dist;	/* distance travelled */
	long nrgy;	/* energy expended */
	long dam;
	long tmp;

	/* convert to relative coordinates (nrgy should be ignored) */
	if (abs_rel(CurrPl, &dr, &dc, &ship, &nrgy) < 0) {
		/* print ship name expecting error msg to follow */
		if (Uerror == UEISEMPTY || Uerror == UEISDEAD) {
			error(ship->name);
		} else
			error("args invalid");
		return(1);	/* game continues */
	}
	dist = MAX(abs(dr), abs(dc));
	if (dist > MRANGE || (tok == IMPULSE && dist > 1)) {
		Uerror = UETOOFAR;
		error("");
		return(1);
	}

	if (tok == MOVE) {
		if (damaged(DWARP, CurrPl, &dam)) /* 0 if device usable */
			return(critmsg(CurrPl, DWARP));
		/* if warp engines damaged, can't go faster than warp 3 */
		if (dam > 0 && dist > 3)
			return(pmsg(UmTEXT, CurrPl
				, "warp damage, max warp 3\n", -1)); 
	} else {
		if (damaged(DIMPULS, CurrPl, &dam)) /* 0 if device usable */
			return(critmsg(CurrPl, DIMPULS));
		/* if impulse engines usable, no effect */
	}

	if (tok == MOVE) {
		/* damage to computer or warp engine affects navigation */
		adjcoords(CurrPl, DWARP, dam, &dr, &dc);
		/* will recalculate dist again later */
	}

	/*
	 * chance of engine damage 50% at warp MRANGE,
	 * 25% at warp MRANGE-1.
	 */
	/* see if the engines got damaged */
	if (dist > MRANGE-2) {
		tmp = 0;
		if (dist == MRANGE)
			tmp = 2;
		else if (dist == MRANGE-1)
			tmp = 1;
		if (rndrange(0, 3) < tmp) {
			if ((CurrPl->flags & OSHORT) == 0)
if (pmsg(UmTEXT, CurrPl
	, "\"Cap'n, me engines canna take much more o' this!!\"\n", -1) < 0)
				return(-1);
		} else {
			/* engines damaged! */
			CurrPl->damdev[DWARP] +=
				rndrange(0, (int)devinfo[DWARP].maxdmg) * tmp;
			if (pmsg(UmTEXT, CurrPl, "Warp Engines damaged!\n", -1)
								< 0)
				return(-1);
			if (CurrPl->damdev[DWARP] > 3*devinfo[DWARP].maxdmg) {
if (pmsg(UmTEXT, CurrPl, "\"Me bairns!  Me poor, poor bairns!\"\n", -1) < 0)
				return(-1);
			}
		}
	}

	/* we're not docked anymore if we were! */
	CurrPl->flags &= ~DOCKED;

	or = CurrPl->rpos;
	oc = CurrPl->cpos;
	switch(direct(or, oc, dr, dc, &br, &bc, &sr, &sc, -1, 0)) {
	case HITOBJ:
		CurrPl->rpos = sr;
		CurrPl->cpos = sc;
		Uerror = UECOLLIDE;
		error("");
		break;

	case HITEDGE:
		CurrPl->rpos = sr;
		CurrPl->cpos = sc;
		Uerror = UELEVGAL;
		error("");
		break;

	case TOOFAR:	/* should never happen; tested above */
		Uerror = UETOOFAR;
		error("");
		return(1);

	default:	/* normal move, no problem */
		CurrPl->rpos = br;
		CurrPl->cpos = bc;
		break;
	}

	/* if we moved, update the map */
	if (CurrPl->rpos != or || CurrPl->cpos != oc) {
		map[CurrPl->rpos][CurrPl->cpos] = map[or][oc];
		needscan(CurrPl->rpos, CurrPl->cpos);
		map[or][oc] = EMPTY;
		needscan(or, oc);
		dr = abs(CurrPl->rpos - or);
		dc = abs(CurrPl->cpos - oc);
	} else
		dr = dc = 0;

	if ((dist = MAX(dr, dc)) > 0) {
		if (dist > 1 && psleep(CurrPl, PMOVE + baudincr) < 0)
			return(-1);
		nrgy = MOVNRGY * dist * dist;
		if (CurrPl->flags & SHIELDED)
			nrgy <<= 1;
		return(decnrgy(CurrPl, nrgy));
	}
	return(1);
}
