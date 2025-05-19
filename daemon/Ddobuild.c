#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "u.h"

extern struct stats stats;
extern int baudincr;
extern PLAYER *CurrPl;
extern short **map;
extern int Mrows, Mcols;
extern PLANET *planets;
extern int Uerror;

void error(), scoreit(), needscan();

/*
 * Capture or build a port.  Do some parsing: syntax of arguments
 * remaining in cmdp is:
 *
 *	[Absolute|Relative|Computed] <vpos> <hpos>
 *
 * We call direct() to see if object is there to work on.
 */
dobuild(tok)
int tok;
{
	char buf[16];
	short dr, dc;	/* distance row and col */
	short or, oc;	/* original row and col */
	short sr, sc;	/* stuck row and col (if necessary) */
	short br, bc;	/* destination (or blocked) row and col */
	STARSHIP *ship;
	PLANET *pt;
	int myside;
	short dist;	/* distance travelled */
	long nrgy;	/* energy expended */
	int tokwait;

	/*
	 * convert to relative coordinates (ship should be set to NULL
	 * and nrgy should be set to -1).
	 */
	if (abs_rel(CurrPl, &dr, &dc, &ship, &nrgy) < 0) {
		error("args invalid");
		return(1);	/* game continues */
	}
	if (ship || nrgy != -1) {
		Uerror = UEBADTOK;
		error("");
		return(1);
	}

	br = CurrPl->rpos + dr;
	bc = CurrPl->cpos + dc;
	if (br < 1 || br > Mrows || bc < 1 || bc > Mcols) {
		Uerror = UEOUTGAL;
		error("");
		return(1);
	}

	dist = MAX(abs(dr), abs(dc));
	if (dist > 1)
		return(pmsg(UmTEXT, CurrPl, "Must be adjacent to planet\n", -1));

	or = CurrPl->rpos;
	oc = CurrPl->cpos;
	if (direct(or, oc, dr, dc, &br, &bc, &sr, &sc, -1, 0) == HITOBJ) {
		/*
		 * There's an object at br,bc -- check it out.
		 * If it's a planet, do something.  If it's a base,
		 * "refuse to surrender" if enemy.
		 * If enemy, tok better be CAPTURE.
		 * If friend, tok must be BUILD.
		 */
		if (OBJTYPE(map[br][bc]) != PORT)
			return(pmsg(UmTEXT, CurrPl
				, "Can't capture/build those!\n", -1));
		/* ok, we got a port. */
		pt = &planets[OBJINDX(map[br][bc])];
#ifdef DEBUG
printf("%c tries to build/capture planet number %d at %d,%d\n",
		CurrPl->ship->name[0], OBJINDX(map[br][bc]), br, bc);
printf("\tplanet's flags=%o octal, builds=%d\n", pt->flags, pt->builds);
#endif
		myside = CurrPl->flags & ALIMASK;
		if (tok == CAPTURE) {
			if (myside == (pt->flags & ALIMASK))
				return(pmsg(UmTEXT, CurrPl
					, "Port already captured!\n", -1));
			else if (pt->flags & ALIMASK) {	/* enemy */
				if (pt->builds == 5) {
					if (pmsg(UmTEXT, CurrPl
						, "Base refuses to surrender!\n"
						, -1) < 0)
						return(-1);

				} else if (--pt->builds == -1) {
					scoreit(0L, SCAPL, 2, 0L, CurrPl
						, (PLAYER *)NULL, pt);
					if (pmsg(UmTEXT, CurrPl
						, "Planet captured!\n", -1) < 0)
						return(-1);
					pt->flags &= ~ALIMASK;
					pt->flags |= myside;
					pt->builds = 0;
					++stats.nplan[myside];
					if (myside == FEDDIE)
						--stats.nplan[EMPIRE];
					else
						--stats.nplan[FEDDIE];

					/* game could end */
					if (stats.nbase[myside] +
						stats.nplan[myside] ==
						stats.nport)
						return(0);
					needscan(pt->rpos, pt->cpos);

					/* wait time */
					tokwait = PCAPTURE + baudincr;
					if (psleep(CurrPl, tokwait) < 0)
						return(-1);
					return(decnrgy(CurrPl, CAPNRGY));
				}
				/* not enough to capture planet */
				if (pmsg(UmTEXT, CurrPl
					, "Port resists capture!\n", -1) < 0)
					return(-1);
				/* wait time */
				tokwait = PCAPTURE + baudincr;
				if (psleep(CurrPl, tokwait) < 0)
					return(-1);

				if (decnrgy(CurrPl, CAPNRGY) < 0)
					return(-1);
				return(potshot(pt, 1, CurrPl));
			}
			/* neutral */
			scoreit(0L, SCAPL, 2, 0L, CurrPl, (PLAYER *)NULL, pt);
			if (pmsg(UmTEXT, CurrPl, "Planet captured!\n", -1) < 0)
				return(-1);
			pt->flags |= myside;
			pt->builds = 0;
			++stats.nplan[myside];
			--stats.nplan[0];
			needscan(pt->rpos, pt->cpos);

			/* game could end */
			if (stats.nbase[myside] + stats.nplan[myside] ==
				stats.nport)
				return(0);
			return(decnrgy(CurrPl, CAPNRGY));
		}

		/* tok == BUILD */
		if (myside != (pt->flags & ALIMASK))
			return(pmsg(UmTEXT, CurrPl
				, "Must capture first!\n", -1));
		/* friendly port */
		if (pt->builds == 5) {
			return(pmsg(UmTEXT, CurrPl
				, "Base already completed!\n"
				, -1));

		} else if (pt->builds + 1 == 5) {
			if (stats.nbase[myside] >= MAXBASES)
				return(pmsg(UmTEXT, CurrPl
					, "All bases operational\n", -1));
			pt->shields = MAXBSHIE;
			--stats.nplan[myside];
			++stats.nbase[myside];
			needscan(pt->rpos, pt->cpos);
			/* NOTE: game can't end as result of build */

			if (pmsg(UmTEXT, CurrPl
				, "Base completed\n", -1) < 0)
				return(-1);
			scoreit(0L, SBASBLT, 2, 0L, CurrPl
					, (PLAYER *)NULL, pt);
		} else {
			(void)sprintf(buf, "%d build%s\n", pt->builds+1,
				(!pt->builds) ? "" : "s");
			if (pmsg(UmTEXT, CurrPl, buf, -1) < 0)
				return(-1);
		}
		if (++pt->builds > 2)
			/* to redraw exclamation pts */
			needscan(pt->rpos, pt->cpos);

		/* wait time */
		tokwait = PBUILD + baudincr;
		if (psleep(CurrPl, tokwait) < 0)
			return(-1);
		return(decnrgy(CurrPl, CAPNRGY));
	}
	return(pmsg(UmTEXT, CurrPl, "Nothing at those coordinates\n", -1));
}
