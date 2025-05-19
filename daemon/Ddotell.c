#include <stdio.h>
#include "config.h"
#include "u.h"

extern int Uerror;
extern PLAYER *CurrPl;
extern char *cmdp;
extern int Mrows, Mcols;

void error(), unlist(), skiptok();

/*
 * dotell -- tell certain players a message.
 * RADIO doesn't need to be on to work, although it has to be
 * in working order.
 */
dotell()
{
	int kflag;	/* store keywords */
	int kword;	/* current keyword */
	int range;	/* range of command */
	STARSHIP *ship;
	int shflag;	/* set if we find any ships */
	long dam;

	if (damaged(DRADIO, CurrPl, &dam)) /* 0 if device usable */
		return(critmsg(CurrPl, DRADIO));

	range = MAX(Mrows, Mcols);
	kflag = 0;
	shflag = 0;
	for (;;) {
		switch(kword = getkword(&ship)) {
		case KTARGETS:	/* targets (enemies in range) */
			kflag |= KENEMY;
			range = PRANGE;
			break;

		case KCLOSEST:
		case KSHIPS:
		case KFEDERATION:	/* Also KHUMAN */
		case KEMPIRE:		/* Also KKLINGON */
		case KFRIENDLY:
		case KENEMY:
		case KALL:	/* both sides */
			kflag |= kword;
			break;

		case -4:	/* semicolon */
			if (kflag == 0 && shflag == 0)
				kflag = KALL;	/* default to all */
			return(tellthem(kflag, range));

		case -5:	/* ship found */
			/* print ship name expecting error msg to follow */
			shflag = 1;
			if (Uerror == UEISEMPTY || Uerror == UEISDEAD) {
				error(ship->name);
				break;
			}
			ship->flags |= RADIO;	/* in a minute they will
						   get a msg */
			break;

		case -1:	/* next token numeric */
			/*
			 * If more than one range is specified, the
			 * user didn't read the directions.  Overwrite
			 * the first with the second.  Also, change
			 * negative ranges to positive 1.
			 */
			(void)sscanf(cmdp, "%d", &range);
			if (range < 0)
				range = 1;
			skiptok();
			break;

		/* the rest are bad keywords */
		case KBASES:
		case KPLANETS:
		case KPORTS:	/* ports (both planets and bases) */
		case KNEUTRAL:
		case KCAPTURED:	/* captured planets */
		case KSUMMARY:	/* count -- don't list locations */
		case -2:	/* no more tokens */
		case -3:	/* Bad token */
		default:
			/* no msgs to send */
			unlist(KSHIPS);
			Uerror = UEBADTOK;
			error("missing semicolon or");
			return(1);
		}
	}
}
