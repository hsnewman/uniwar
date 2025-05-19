#include <stdio.h>
#include "config.h"
#include "u.h"

extern int Uerror;
extern char *cmdp;
extern int Mrows, Mcols;

void error(), skiptok();

/*
 * dobapl -- list info about bases or planets.
 */
dobapl(kflag)
int kflag;	/* store keywords */
{
	int kword;	/* current keyword */
	int range;	/* range of command */
	STARSHIP *ship;
	int adjs;

	range = MAX(Mrows, Mcols);
	for (;;) {
		switch(kword = getkword(&ship)) {
		case KTARGETS:	/* targets (enemies in range) */
			kflag |= KENEMY;
			range = PRANGE;
			break;

		case KBASES:
			if (kflag & KPLANETS)
				goto badkword;
			break;

		case KCAPTURED:	/* captured planets */
		case KPLANETS:
			if (kflag & KBASES)
				goto badkword;
			break;

		case KNEUTRAL:
			if (kflag & KBASES)
				goto badkword;
			/* fall through */

		case KCLOSEST:
		case KFEDERATION:	/* Also KHUMAN */
		case KEMPIRE:		/* Also KKLINGON */
		case KFRIENDLY:
		case KENEMY:
		case KSUMMARY:	/* count -- don't list locations */
			kflag |= kword;
			break;

		case -2:	/* no more tokens */
			/*
			 * No adjective means all adjectives.
			 * We got a noun - either bases or planets.
			 */
			adjs = kflag & ADJS;
			/*
			adjs = kflag & (KALL|KNEUTRAL|KFRIENDLY|KENEMY);
			*/
			if (!adjs)
				kflag |= (KALL|KNEUTRAL);
			return(listthem(kflag, range));

		case -1:	/* next token numeric */
			/*
			 * We don't allow "list rpos cpos" in this version.
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
		case KSHIPS:
		case KPORTS:	/* ports (both planets and bases) */
		case KALL:	/* both sides */
		case -3:	/* Bad token */
		case -4:	/* semicolon */
		case -5:	/* ship found */
		default:
badkword:
			/* no msgs to send */
			Uerror = UEBADTOK;
			error("missing semicolon or");
			return(1);
		}
	}
}
