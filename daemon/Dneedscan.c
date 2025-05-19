#include "config.h"
#include "u.h"

extern short usedship;
extern PLAYER *CurrPl;
extern PLAYER ships[];

/*
 * needscan -- all ships within PRANGE of (r, c) need to do
 * a scan (map[r][c] just changed).  Flag them as such for
 * later when allscan() is called.
 */
void
needscan(r, c)
register short r;
register short c;
{
	register PLAYER *pl;
	short i;

	for (i = usedship; i != -1; i = pl->list) {
		pl = &ships[i];
		if (MAX(abs(pl->rpos - r), abs(pl->cpos - c)) <= PRANGE)
			pl->flags |= DOSCAN;
	}
}

/*
 * allscan -- all ships with DOSCAN flag turned on get a scan done.
 * this flag is set when the map changes in range of the ship.
 * Don't check for error on scan for now.
 *
 * No output will actually go to the player if SCANON is off -- this
 * check is made in doscan().
 */
allscan()
{
	PLAYER *savPl;
	register short i;

	/* Scan for all players whose viewing area was affected */
	savPl = CurrPl;
	for (i = usedship; i != -1; i = CurrPl->list) {
		CurrPl = &ships[i];
		if ((CurrPl->flags & DOSCAN) &&
		    (CurrPl->flags & ACTIVE) && (CurrPl->flags & DIED) == 0) {
			if (CurrPl->flags & SCANON)
				CurrPl->flags &= ~DOSCAN;
			/*
			 * otherwise, leave DOSCAN on so will be done when
			 * SCANON is enabled.
			 */

			(void)doscan();
		}
	}
	CurrPl = savPl;
	return(1);
}
