#include <stdio.h>
#include "config.h"
#include "u.h"

extern short usedship;
extern PLAYER *CurrPl;
extern PLAYER ships[];
extern char *cmdp;
extern int Mrows, Mcols;

/*
 * tellthem -- tell msg in cmdp to the players indicated
 * by flags that are in 'range'.  Also tell them to whom the
 * message went.
 *
 * When range is between PRANGE and maximum range, replace
 * the list of sendees with a list of friendly sendees and
 * the specified range.
 */
tellthem(flags, range)
int flags;
int range;
{
	register int i;
	int clflag;	/* if we need the closest */
	int myside;
	int aliflags;
	int count;	/* number of people to get msg */
	int odist, dist;
	int rangeflag;	/* set if need to ignore enemies out of PRANGE */
	register PLAYER *pl, *clpl;
	register STARSHIP *sh;
	char buf[256];	/* for message */
	char buf2[128];	/* for error message if necessary */
	char buf3[3*MAXPL];	/* for ships getting msg */

	if (range > PRANGE && range != MAX(Mrows, Mcols))
		rangeflag = 1;
	else
		rangeflag = 0;
	if (flags & KCLOSEST)
		clflag = 1;
	else
		clflag = 0;

	/* Only interested in sides now */
	flags &= (KFEDERATION|KEMPIRE|KFRIENDLY|KENEMY);
	myside = (CurrPl->flags & ALIMASK);
	aliflags = 0;

	/* if something was specified, go by it */
	if (flags) {
		if (flags & KFRIENDLY)
			aliflags |= myside;
		if (flags & KENEMY) {
			if (myside == EMPIRE)
				aliflags |= FEDDIE;
			else
				aliflags |= EMPIRE;
		}
		if (flags & KFEDERATION)
			aliflags |= FEDDIE;
		if (flags & KEMPIRE)
			aliflags |= EMPIRE;
	}

	/* all set to choose who to send to */
	clpl = (PLAYER *)NULL;
	odist = range+1;
	count = 0;
	for (i = usedship; i != -1; i = pl->list) {
		pl = &ships[i];
		if ((pl->flags & ACTIVE) == 0)
			continue;	/* player in pregame */

		sh = pl->ship;
		if (sh->flags & RADIO) {	/* already set */
			count++;
			continue;
		}

		/* is player in specified range */
		if ((dist = MAX(abs(CurrPl->rpos - pl->rpos),
			abs(CurrPl->cpos - pl->cpos))) > range)
			continue;

		/* is player on proper alliance */
		if ((sh->flags & aliflags) == 0)
			continue;

		/* is player closest (if necessary) */
		if (clflag) {
			/* somebody closer or talking about self */
			if ((clpl && dist > odist) || pl == CurrPl)
				continue;
			else {
				clpl = pl;
				odist = dist;
				count++;	/* got one */
			}
		/* got somebody that meets all the requirements. */
		} else {
			sh->flags |= RADIO;
			count++;
		}
	}

	/* error case */
	if (count == 0)
		return(pmsg(UmTEXT, CurrPl, "No message sent\n", -1));

	if (clflag)
		clpl->ship->flags |= RADIO;

	/*
	 * first pass to construct list of ships.  There must be at
	 * least one since count was positive
	 */
	*buf3 = '\0';
	if (rangeflag)	/* range between PRANGE and max specified */
		(void)sprintf(buf3, "(+ range %d) ", range);
	for (i = usedship; i != -1; i = pl->list) {
		pl = &ships[i];
		if ((pl->flags & ACTIVE) == 0)
			continue;	/* player in pregame */
		/*
		 * Don't include enemy ships out of my range in the list.
		 * That would be cheating to locate enemy ships!
		 */
		if (rangeflag && dist > PRANGE && (sh->flags & ALIMASK)!=myside)
			continue;

		if (pl->ship->flags & RADIO)
			(void)sprintf(&buf3[strlen(buf3)]
				, "%c ", *pl->ship->name);
	}
	buf3[strlen(buf3)-1] = ':';

	/* second pass to send the messages */
	*buf2 = '\0';	/* list of ships we can't raise */
	for (i = usedship; i != -1; i = pl->list) {
		pl = &ships[i];
		if ((pl->flags & ACTIVE) == 0)
			continue;	/* player in pregame */
		sh = pl->ship;
		/* later check to see if gagged against CurrPl */
		if (sh->flags & RADIO) {
			if ((pl->flags & RADIO) == 0) {
				(void)sprintf(&buf2[strlen(buf2)]
					, "%c ", *pl->ship->name);
			} else {
				if (pl->flags & OSHORT)
(void)sprintf(buf, "<%c> to %s %s\n", CurrPl->ship->name[0], buf3, cmdp);
				else
(void)sprintf(buf, "<%s/%s> to %s %s\n", CurrPl->name, CurrPl->ship->name
					, buf3, cmdp);
				if (pmsg(UmTEXT, pl, buf, -1) < 0)
					return(-1);
			}
			sh->flags &= ~RADIO;
		}
	}
	if (*buf2) {	/* we couldn't reach some players */
		(void)sprintf(buf2, "can't raise %s\n", buf2);
		return(pmsg(UmTEXT, CurrPl, buf2, -1));
	}
	return(1);
}
