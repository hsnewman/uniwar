#include <stdio.h>
#include "config.h"
#include "u.h"

extern int Uerror;

void error(), skiptok();

/*
 * dotarg -- list info about targets.
 */
dotarg()
{
	int kflag;	/* store keywords */
	int kword;	/* current keyword */
	int range;	/* range of command */
	STARSHIP *ship;
	int nouns;

	range = PRANGE;
	kflag = KENEMY;
	for (;;) {
		switch(kword = getkword(&ship)) {
		case KTARGETS:	/* targets (enemies in range) */
		case KENEMY:
			break;

		case KBASES:
		case KPLANETS:
		case KPORTS:	/* ports (both planets and bases) */
		case KCLOSEST:
		case KSHIPS:
		case KSUMMARY:	/* count -- don't list locations */
			kflag |= kword;
			break;

		case -2:	/* no more tokens */
			/*
			 * No noun means all nouns.
			 * We got an adjective - ENEMY.
			 */
			nouns = kflag & NOUNS;
			/*
			nouns = kflag & (KSHIPS|KPORTS);
			*/
			if (!nouns)
				kflag |= NOUNS;
			return(listthem(kflag, range));

		/* the rest are bad keywords */
		case -1:	/* next token numeric */
			skiptok();
			/* fall through */
		case KCAPTURED:	/* captured planets */
		case KNEUTRAL:
		case KFEDERATION:	/* Also KHUMAN */
		case KEMPIRE:		/* Also KKLINGON */
		case KFRIENDLY:
		case KALL:	/* both sides */
		case -3:	/* Bad token */
		case -4:	/* semicolon */
		case -5:	/* ship found */
		default:
			/* no msgs to send */
			Uerror = UEBADTOK;
			error("missing semicolon or");
			return(1);
		}
	}
}
