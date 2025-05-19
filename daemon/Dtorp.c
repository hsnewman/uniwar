#include <stdio.h>
#include "config.h"
#include "u.h"

extern struct devinfo devinfo[];
extern int baudincr;
extern PLAYER *CurrPl;
extern PLAYER ships[];
extern short **map;
extern PLANET *planets;

char *coordstr();
void adjcoords(), needscan();
long time();

/*
 * torp -- fire a burst of torps aimed at r,c relative to pl.
 * burst is the number of torps to fire.
 */
torp(pl, burst, r, c)
PLAYER *pl;
long burst;
short r, c;
{
	register int i;
	short tr, tc;
	short sr, sc;	/* stuck row and col (if necessary) */
	short br, bc;	/* destination (or blocked) row and col */
	short maxmove;
	short mapspot;
	short obx;
	int myside;
	int abortflg;	/* if burst gets aborted */
	int what;
	char buf[64];
	PLANET tpt;
	long hit, dam;
	long tmp;	/* temporaries */
	int ret;
	PLANET *pt1, *pt2;
	PLAYER *pl2;
	int sleeptime;
	long thistime;

	if (damaged(DTUBES, pl, &dam)) /* 0 if device usable */
		return(critmsg(pl, DTUBES));
	if (pl->torps < burst)
		return(pmsg(UmTEXT, pl, "Insufficient torps for burst!\n", -1));

	/*
	 * Torpedoes minimum range is TRANGE, but may actually
	 * travel 1 or 2 spaces farther.  The probability is 1/2
	 * that it will only go TRANGE, 1/4 that it will go TRANGE+1,
	 * and 1/4 that it will go TRANGE+2.
	 *
	 * Torpedoes may be off target.  Computer damage, torpedo
	 * damage, and misfires may contribute to this.  Players
	 * will find that close range shots not only are more
	 * accurate, but that they tend to stay on course once fired
	 * (see adjcoords()).
	 *
	 * Misfires happen 5% of the time.  The remainder of the burst
	 * is aborted (the torps are lost), and 33% of the time the
	 * torpedo tubes are damaged as well (between 1 and 2*maxdmg!).
	 *
	 * You can't hit friendly ships and ports.  You can destroy
	 * planets and cause stars to go nova.
	 *
	 * The target's shield percent determines whether or not
	 * a phaser bolt will be deflected.  The probability is
	 * linear (100% shields, 90% chance of deflection; 10% shields,
	 * 0% chance of deflection).  A deflected torp does between
	 * 0 and 50 units of damage to the victim.
	 *
	 * A torpedo hit causes 400-600 damage to target.  Probabilites
	 * of removing builds from planets are discussed in dlvrhit().
	 */

	what = TORP;

	/*
	 * sleeptime depends on last time torps were fired
	 * and also on damage.
	 */

	/* torps needed this long to be reloaded */
	sleeptime = burst * (PTORPEDO + baudincr + pl->damdev[DTUBES]/10000);

	/* torps already loaded for (thistime - pl->totime) seconds */
	thistime = time(0L);
	sleeptime -= (thistime - pl->totime);
	if (sleeptime > 0) {
		/* must wait to do command */
		/* if 2 is returned, no sleep was needed */
		if ((ret = psleep(pl, sleeptime)) != 2) {
			/* command must be reparsed after sleep */
			pl->plbuf.start = pl->plbuf.ostart;
			return(ret);
		}
	}
	/* can do command now */
	pl->totime = thistime;

	/* Fire the burst */
	if ((pl->flags & DOCKED) == 0)
		/* if docked, infinite supply of torps. */
		pl->torps -= burst;
	abortflg = 0;
	for (i = 1; abortflg == 0 && i <= burst; i++) {
		/* damage may cause course adjustment to this torp */
		tr = r;
		tc = c;
		adjcoords(pl, DTUBES, dam, &tr, &tc);

		if (rndrange(0, 99) < 5) {	/* we misfired */
			(void)sprintf(buf, "Torpedo %d misfires!\n", i);
			abortflg = 1;	/* abort rest of torps */
			if (pmsg(UmTEXT, CurrPl, buf, -1) < 0)
				return(-1);

			if (rndrange(0, 99) < 33) {
				/* torps damaged! */
				pl->damdev[DTUBES] += rndrange(1
					, (int)devinfo[DTUBES].maxdmg * 2);
				if ((pl->flags & OSHORT) == 0) {
					if (pmsg(UmTEXT, pl
						, "Scotty: \"$#*&!\"\n"
							, -1) < 0)
						return(-1);
				}
				if (pmsg(UmTEXT, pl, "Torpedo tubes damaged!\n"
								, -1) < 0)
					return(-1);
			}
		}

		/* how far to send this torp? */
		maxmove = 0;
		switch (rnd() % 4) {
		case 0:
			maxmove++;
			/* fall thru */
		case 1:
			maxmove++;
			/* fall thru */
		default:
			maxmove += TRANGE;
			break;
		}

		/* torpedo away!  Let's see what we hit. */
		switch (direct(CurrPl->rpos, CurrPl->cpos, tr, tc, &br, &bc
			, &sr, &sc, maxmove, 0)) {
		case HITEDGE:
			br = sr;
			bc = sc;
			/* fall thru */
		case 0:
		default:
			/* we missed -- torp exploded at (sr,sc) */
			if (pl->flags & OSHORT)
				(void)sprintf(buf, "T%d miss %s\n",
					i, coordstr(pl, br, bc));
			else
				(void)sprintf(buf, "Torpedo %d missed %s\n",
					i, coordstr(pl, br, bc));
			if (pmsg(UmTEXT, pl, buf, -1) < 0)
				return(-1);
			break;

		case HITOBJ:
			/* find out what's out there */
			myside = pl->flags & ALIMASK;
			mapspot = map[br][bc];
			obx = OBJINDX(mapspot);

			switch(OBJTYPE(mapspot)) {
			case STAR:	/* did it nova? */
				tpt.rpos = br;
				tpt.cpos = bc;
				if (pl->flags & OSHORT)
					(void)sprintf(buf, "T%d hit * %s\n",
						i, coordstr(pl, br, bc));
				else
					(void)sprintf(buf
						, "Torpedo %d hit star %s\n",
						i, coordstr(pl, br, bc));
				if (pmsg(UmTEXT, pl, buf, -1) < 0)
					return(-1);
				if (rnd() % 2) {
					/*
					 * blow up star and cause appropriate
					 * chain reactions.  Game could end.
					 */
					if ((ret = nova(&tpt)) <= 0)
						return(ret);
				}
				break;

			case PORT:
				pt2 = &planets[obx];
				if ((pt2->flags & ALIMASK) == myside) {
(void)sprintf(buf, "Torpedo %d neutralized %s\n", i, coordstr(pl, br, bc));
					
					if (pmsg(UmTEXT, pl, buf, -1) < 0)
						return(-1);
					break;
				}

				/* was it deflected? */
				if (pt2->builds == 5) {
					tmp = (pt2->shields * 100)/MAXBSHIE;
					tmp = MAX(tmp-10, 0);
					if (rndrange(0,99) < tmp)
						/* deflected */
						hit = rndrange(0, 5000);
					else
						hit = rndrange(40000, 60000);
					/* note - can't displace bases */
				}
				/*
				 * if pt2 is a planet, dlvrhit handles
				 * probability of knocking builds off.
				 * (pl2 and pt1 are always ignored)
				 */
				pl2 = (PLAYER *)NULL;
				pt1 = (PLANET *)NULL;
				if ((ret = dlvrhit(what, pl, pl2, pt1
					, pt2, hit, 2)) <= 0)
					return(ret);
				break;	/* next torp */

			case SHIP:
				pl2 = &ships[obx];
				if ((pl2->flags & ALIMASK) == myside) {
(void)sprintf(buf, "Torpedo %d neutralized %s\n", i, coordstr(pl, br, bc));
					
					if (pmsg(UmTEXT, pl, buf, -1) < 0)
						return(-1);
					break;
				}

				/* was it deflected? */
				if (pl2->flags & SHIELDED) {
					tmp = (pl2->shields * 100)/MAXSHIE;
					tmp = MAX(tmp-10, 0);
				} else
					tmp = 0;
				if (tmp && rndrange(0,99) < tmp)
					/* deflected */
					hit = rndrange(0, 5000);
				else {
					hit = rndrange(40000, 60000);
					/*
					 * displace the ship.  Use tr,tc
					 * because we need br,bc to clear
					 * map[][].
					 */
					ret = direct(br, bc, br-sr, bc-sc
						, &tr, &tc, &sr, &sc, 1, 0);
					if (ret == 0) {	/* was displaced */
						what |= DISPLAC;
						pl2->rpos = tr;
						pl2->cpos = tc;
						map[tr][tc] = map[br][bc];
						needscan(tr, tc);
						map[br][bc] = EMPTY;
						needscan(br, bc);
						br = tr;
						bc = tc;
					}
				}
				/* pl hits pl2 */
				if ((ret = dlvrhit(what, pl, pl2, pt1
					, pt2, hit, 1)) <= 0)
					return(ret);
				break;	/* next torp */
			}
			break;
		}
	} /* end of "for each torp in burst" */

	return(1);
}
