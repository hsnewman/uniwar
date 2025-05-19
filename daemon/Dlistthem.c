#include <stdio.h>
#include "config.h"
#include "u.h"

#define plural(x) ((x) == 1 ? "" : "s")

extern struct stats stats;
extern short usedship;
extern PLAYER *CurrPl;
extern PLAYER ships[];
extern PLANET *planets;
extern char Outbuf[];

void cpsht(), cplng(), ghitr();
void unlist();
char *strcat(), *memcpy();
char *known();

static char *noun[] = {
	"forces",
	"ships or bases",
	"ships or planets",
	"ships",
	"ports",
	"bases",
	"planets",
};
/* indices to noun[] array */
#define	XFORCE	0
#define	XSH_BA	1
#define	XSH_PL	2
#define	XSHIPS	3
#define	XPORTS	4
#define	XBASES	5
#define	XPLANS	6

/*
 * listthem -- tell CurrPl about requested objects.
 *
 * If we're summarizing, ignore range and "closest".
 *
 * Messages have headers of the form: mwwffffrrcc
 *	m is char message type (UmLIST).
 *	ww is short "what happened".
 *	ffff is long, player flags.
 *	rr is short row psn of player getting the msg.
 *	cc is short col psn of player getting the msg.
 *
 */
listthem(flags, range)
int flags;
int range;
{
	register int i;
	int what;
	long hit;
	int clflag;	/* if we need the closest */
	int cltype;
	int myside, myknown;
	int msize;
	int aliflags;
	int count;	/* number of people to get msg */
	int odist, dist;
	int sumflag;
	PLANET *pt, *clpt;
	register PLAYER *pl, *clpl;
	register STARSHIP *sh;
	char buf[MAXMSG];	/* for message */
	char buf2[128];	/* for error message if necessary */
	char *hdr, *txt;
	int nouncase;

	if (CurrPl->flags & ACTIVE) {
		/* not in pregame, print extra ----- */
		if (pmsg(UmTEXT, CurrPl, "-----\n", -1) < 0)
			return(-1);
	}
				
	if (flags & KSUMMARY)
		sumflag = 1;
	else
		sumflag = 0;

	/* summary overrides "closest" */
	if ((flags & KCLOSEST) && !sumflag)
		clflag = 1;
	else
		clflag = 0;

	/* "closest" -- we print enemy object even if "targets" was specified */
	if (clflag)
		flags &= ~KNOOUTRNG;

	/* temporarily use myside to adjust flags */
	if ((CurrPl->flags & ALIMASK) == FEDDIE)
		myside = KFEDERATION;
	else
		myside = KEMPIRE;

	if (flags & KFRIENDLY)
		flags |= myside;
	if (flags & KENEMY) {
		if (myside == KEMPIRE)
			flags |= KFEDERATION;
		else
			flags |= KEMPIRE;
	}
	/* clear KFRIENDLY and KENEMY */
	flags &= ~(KFRIENDLY|KENEMY);

	/* now set myside to FEDDIE or EMPIRE */
	myside = (CurrPl->flags & ALIMASK);
	if (myside == FEDDIE)
		myknown = FKNOWS;
	else
		myknown = EKNOWS;

	aliflags = 0;
	if (flags & KFEDERATION)
		aliflags |= FEDDIE;
	if (flags & KEMPIRE)
		aliflags |= EMPIRE;
	if (flags & KNEUTRAL)
		aliflags |= KNEUTRAL;

	/* if summary, we're just counting.  Otherwise, list info */
	if (sumflag) {
		*buf = '\0';
		if (flags & KSHIPS) {
			if ((flags & KFEDERATION) && stats.Fcnt)
				(void)sprintf(&buf[strlen(buf)],
					"%d Federation ship%s\n",
					stats.Fcnt, plural(stats.Fcnt));
			if ((flags & KEMPIRE) && stats.Ecnt)
				(void)sprintf(&buf[strlen(buf)],
					"%d Empire ship%s\n",
					stats.Ecnt, plural(stats.Ecnt));
			if (stats.Fcnt || stats.Ecnt)
				(void)strcat(buf, "\n");
		}
		if (flags & KBASES) {
			if ((flags & KFEDERATION) && stats.nbase[FEDDIE])
				(void)sprintf(&buf[strlen(buf)],
					"%d Federation base%s\n",
					stats.nbase[FEDDIE],
					plural(stats.nbase[FEDDIE]));
			if ((flags & KEMPIRE) && stats.nbase[EMPIRE])
				(void)sprintf(&buf[strlen(buf)],
					"%d Empire base%s\n",
					stats.nbase[EMPIRE],
					plural(stats.nbase[EMPIRE]));
			if (stats.nbase[FEDDIE] || stats.nbase[EMPIRE])
				(void)strcat(buf, "\n");
		}
		if (flags & KPLANETS) {
			if ((flags & KFEDERATION) && stats.nplan[FEDDIE])
				(void)sprintf(&buf[strlen(buf)],
					"%d Federation planet%s\n",
					stats.nplan[FEDDIE],
					plural(stats.nplan[FEDDIE]));
			if ((flags & KEMPIRE) && stats.nplan[EMPIRE])
				(void)sprintf(&buf[strlen(buf)],
					"%d Empire planet%s\n",
					stats.nplan[EMPIRE],
					plural(stats.nplan[EMPIRE]));
			if ((flags & KNEUTRAL) && stats.nplan[0])
				(void)sprintf(&buf[strlen(buf)],
					"%d neutral planet%s\n",
					stats.nplan[0],
					plural(stats.nplan[0]));
		}
		return(pmsg(UmTEXT, CurrPl, buf, -1));
	}

	/* build header */
	msize = -1;
	hdr = buf;
	txt = buf+11;
	*hdr++ = UmLIST;
	cpsht((short)LISTS, &hdr);
	cplng(CurrPl->flags, &hdr);
	cpsht(CurrPl->rpos, &hdr);
	cpsht(CurrPl->cpos, &hdr);

	/* Ok, let's get going.  Order is ships, bases, planets */
	/*
	 * since individual ships might be specified, have to
	 * test (flags & KSHIPS) in the loop.
	 */
	pt = (PLANET *)NULL;
	cltype = -1;
	odist = range+1;
	count = 0;	/* count objs found that we will list. */
	for (i = usedship; i != -1; i = pl->list) {
		pl = &ships[i];
		if ((pl->flags & ACTIVE) == 0)
			continue;	/* player in pregame */

		sh = pl->ship;

		dist = MAX(abs(CurrPl->rpos - pl->rpos),
			abs(CurrPl->cpos - pl->cpos));

		if (sh->flags & RADIO) {	/* already set */
			what = LISTS;
			if (dist > PRANGE && (sh->flags & myside) == 0) {
				/* overrides KNOOUTRNG */
				what |= OUTOFRNG;
				hit = 0;
			} else
				hit = 1;	/* more info to print */

			ghitr(1, pl, pt, hit, what, &txt);
			count++;	/* got one */
			continue;
		}

		if ((flags & KSHIPS) == 0)
			continue;	/* just want ships already set */

		/* is player out of range or on wrong team */
		if (dist > range || (sh->flags & aliflags) == 0)
			continue;

		/* is player closest (if necessary) */
		if (clflag) {
			if (dist <= odist && pl != CurrPl &&
				((sh->flags & myside) || dist < PRANGE)) {
				/* got new closest object */
				cltype = SHIP;
				clpl = pl;
				odist = dist;
				count++;	/* got one */
			}
		} else {	/* valid player */
			what = LISTS;
			if (dist > PRANGE && (sh->flags & myside) == 0) {
				if (flags & KNOOUTRNG)
					continue;
				what |= OUTOFRNG;
				hit = 0;
			} else
				hit = 1;	/* more info to print */

			ghitr(1, pl, pt, hit, what, &txt);
			count++;	/* got one */
			continue;
		}
	}
	unlist(KSHIPS);

	/* error case */
	if (count == 0 && !clflag && (flags & KSHIPS)) {
		if ((aliflags & (FEDDIE|EMPIRE)) == (FEDDIE|EMPIRE))
			(void)sprintf(buf2, "No%s ships\n"
				, known(FEDDIE|EMPIRE, XSHIPS));
		else if (aliflags & FEDDIE)
			(void)sprintf(buf2, "No%s federation ships\n"
				, known(FEDDIE, XSHIPS));
		else if (aliflags & EMPIRE)
			(void)sprintf(buf2, "No%s empire ships\n"
				, known(EMPIRE, XSHIPS));
		if (pmsg(UmTEXT, CurrPl, buf2, -1) < 0)
			return(-1);
	}

	/* ready for BASES */
	pl = (PLAYER *)NULL;
	count = 0;
	if (flags & KBASES) {
		for (i = 0; i < stats.nport; i++) {
			pt = &planets[i];
			/* not a base or I don't know about it */
			if (pt->builds != 5 || (pt->flags & myknown) == 0)
				continue;

			dist = MAX(abs(CurrPl->rpos - pt->rpos),
				abs(CurrPl->cpos - pt->cpos));

			/* is base out of range or on wrong team */
			if (dist > range || (pt->flags & aliflags) == 0)
				continue;

			if (dist > PRANGE && (pt->flags & myside) == 0) {
				/* TA -- we don't list known out-of-range */
				if (flags & KNOOUTRNG)
					continue;
				hit = 0;
			} else
				hit = 1;

			/* is base closest (if necessary) */
			if (clflag) {
				if (dist <= odist) {
					/* got new closest object */
					cltype = PORT;
					clpt = pt;
					odist = dist;
					count++;	/* got one */
				}
			} else {
				ghitr(3, pl, pt, hit, LISTS, &txt);
				count++;	/* got one */
			}
		}

		/* error case */
		if (count == 0 && !clflag) {
			if ((aliflags & (FEDDIE|EMPIRE)) == (FEDDIE|EMPIRE))
				(void)sprintf(buf2, "No%s bases\n"
					, known(FEDDIE|EMPIRE, XBASES));
			else if (aliflags & FEDDIE)
				(void)sprintf(buf2, "No%s federation bases\n"
					, known(FEDDIE, XBASES));
			else if (aliflags & EMPIRE)
				(void)sprintf(buf2, "No%s empire bases\n"
					, known(EMPIRE, XBASES));
			if (pmsg(UmTEXT, CurrPl, buf2, -1) < 0)
				return(-1);
		}
	}

	/* ready for PLANETS */
	count = 0;
	if (flags & KPLANETS) {
		for (i = 0; i < stats.nport; i++) {
			pt = &planets[i];
			/* not a planet or I don't know about it */
			if (pt->builds == 5 || (pt->flags & myknown) == 0)
				continue;

			dist = MAX(abs(CurrPl->rpos - pt->rpos),
				abs(CurrPl->cpos - pt->cpos));

			/* is planet out of range */
			if (dist > range)
				continue;

			/* is planet on the right side? */
			switch(pt->flags & (EMPIRE|FEDDIE)) {
			case EMPIRE:
				if ((aliflags & EMPIRE) == 0)
					continue;
				break;

			case FEDDIE:
				if ((aliflags & FEDDIE) == 0)
					continue;
				break;

			case 0:
				if ((flags & KNEUTRAL) == 0)
					continue;
				break;

			default:
				continue;	/* bad flags! */
			}

			if (dist > PRANGE && (pt->flags & myside) == 0) {
				/* TA -- we don't list known out-of-range */
				if (flags & KNOOUTRNG)
					continue;
				hit = 0;
			} else
				hit = 1;

			/* is planet closest (if necessary) */
			if (clflag) {
				if (dist <= odist) {
					/* got new closest object */
					cltype = PORT;
					clpt = pt;
					odist = dist;
					count++;	/* got one */
				}
			} else {
				ghitr(3, pl, pt, hit, LISTS, &txt);
				count++;	/* got one */
			}
		}

		/* error case */
		if (count == 0 && !clflag) {
			if ((aliflags & (FEDDIE|EMPIRE)) == (FEDDIE|EMPIRE) &&
					(flags & KNEUTRAL))
				(void)sprintf(buf2, "No%s planets\n"
					, known(EMPIRE|FEDDIE, XPLANS));
			else {
				/* didn't want "ALL" */
				if (aliflags & FEDDIE)
					(void)sprintf(buf2
					, "No%s federation planets\n"
					, known(FEDDIE, XPLANS));
				if (aliflags & EMPIRE)
					(void)sprintf(buf2
					, "No%s empire planets\n"
					, known(EMPIRE, XPLANS));
				if (flags & KNEUTRAL)
					(void)sprintf(buf2
					, "No%s neutral planets\n"
					, known(0, XPLANS));
			}
			if (pmsg(UmTEXT, CurrPl, buf2, -1) < 0)
				return(-1);
		}
	}

	if (clflag) {	/* print closest */
		if (cltype == -1) {
			/* couldn't find any! */

			/* first set nouncase to what we couldn't find */
			if (flags & KSHIPS) {
				if (flags & KBASES) {
					if (flags & KPLANETS)
						nouncase = XFORCE;
					else
						nouncase = XSH_BA;
				} else if (flags & KPLANETS)
					nouncase = XSH_PL;
				else
					nouncase = XSHIPS;
			} else if (flags & KBASES) {
				if (flags & KPLANETS)
					nouncase = XPORTS;
				else
					nouncase = XBASES;
			} else if (flags & KPLANETS)
				nouncase = XPLANS;
			else
				/* should never be reached */
				nouncase = XFORCE;

			*buf2 = '\0';
			if (aliflags & FEDDIE)
				(void)sprintf(buf2, "No%s federation %s\n"
					, known(FEDDIE, nouncase)
					, noun[nouncase]);
			if (aliflags & EMPIRE)
				(void)sprintf(&buf2[strlen(buf2)]
					, "No%s empire %s\n"
					, known(EMPIRE, nouncase)
					, noun[nouncase]);
			if ((flags & KNEUTRAL) && (flags & KPLANETS))
				(void)sprintf(&buf2[strlen(buf2)]
					, "No%s neutral planets\n"
					, known(0, nouncase));
			return(pmsg(UmTEXT, CurrPl, buf2, -1));
		}
		/* found one */
		if (odist > PRANGE)
			hit = 0;
		else
			hit = 1;

		switch(cltype) {
		case SHIP:
			if ((clpl->flags & ALIMASK) == myside)
				hit = 1;
			ghitr(1, clpl, pt, hit, LISTS, &txt);
			break;

		case PORT:
			if ((clpt->flags & ALIMASK) == myside)
				hit = 1;
			ghitr(3, pl, clpt, hit, LISTS, &txt);
			break;

		default:
			break;	/* can't happen */
		}
	}
	*txt++ = '\377';	/* end of message */
	msize = txt - buf;	/* do sendcmd() instead of pmsg() */
	if (msize == 12)	/* nothing to say */
		return(1);
	(void)memcpy(Outbuf+sizeof(short), buf, msize);
	return(sendcmd(CurrPl, msize));
}

/*
 * return either " known" or "" depending on whether total objects
 * is positive or 0.
 *
 * ally can be 0 (neutrals), EMPIRE, FEDDIE, or EMPIRE|FEDDIE (interested
 * in all info).
 */
char *
known(ally, nouncase)
int ally;
int nouncase;
{
	int scnt, nbase, nplan;
	int total;

	scnt = nbase = nplan = 0;
	if (ally == FEDDIE) {
		scnt = stats.Fcnt;
		nbase = stats.nbase[ally];
		nplan = stats.nplan[ally];
	} else if (ally == EMPIRE) {
		scnt = stats.Ecnt;
		nbase = stats.nbase[ally];
		nplan = stats.nplan[ally];
	} else if (ally == 0)
		nplan = stats.nplan[0];
	else {
		scnt = stats.Fcnt + stats.Ecnt;
		nbase = stats.nbase[FEDDIE] + stats.nbase[EMPIRE];
		nplan = stats.nplan[FEDDIE] + stats.nplan[EMPIRE]
					+ stats.nplan[0];
	}

	switch(nouncase) {
	default:
	case 0:
		total = scnt + nbase + nplan;
		break;

	case 1:
		total = scnt + nbase;
		break;

	case 2:
		total = scnt + nplan;
		break;

	case 3:
		total = scnt;
		break;

	case 4:
		total = nbase + nplan;
		break;

	case 5:
		total = nbase;
		break;

	case 6:
		total = nplan;
		break;
	}

	if (total)
		return(" known");
	else
		return("");
}
