#include <stdio.h>
#include "config.h"
#include "u.h"

extern int Uerror;
extern int Mrows, Mcols;

void error(), skiptok();

/*
 * dosumm -- list summary info.
 */
dosumm()
{
	int kflag;	/* store keywords */
	int kword;	/* current keyword */
	int range;	/* range of command */
	STARSHIP *ship;
	int adjs, nouns;

	range = MAX(Mrows, Mcols);
	kflag = KSUMMARY;
	for (;;) {
		switch(kword = getkword(&ship)) {
		case KTARGETS:	/* targets (enemies in range) */
		case KENEMY:
			kflag |= KENEMY;
			range = PRANGE;
			break;

		case KSUMMARY:	/* count -- don't list locations */
			break;

		case KBASES:
		case KPLANETS:
		case KPORTS:	/* ports (both planets and bases) */
		case KSHIPS:
		case KCAPTURED:	/* captured planets */
		case KNEUTRAL:
		case KFEDERATION:	/* Also KHUMAN */
		case KEMPIRE:		/* Also KKLINGON */
		case KFRIENDLY:
			kflag |= kword;
			break;

		case -2:	/* no more tokens */
			/*
			 * No adjective means all adjectives.
			 * No noun means all nouns.
			 */
			nouns = kflag & NOUNS;
			adjs = kflag & ADJS;
			/*
			nouns = kflag & (KSHIPS|KPORTS);
			adjs = kflag & (KALL|KNEUTRAL|KFRIENDLY|KENEMY);
			*/
			if (!nouns)
				kflag |= NOUNS;
			if (!adjs)
				kflag |= (KALL|KNEUTRAL);
			return(listthem(kflag, range));

		/* the rest are bad keywords */
		case -1:	/* next token numeric */
			skiptok();
			/* fall through */
		case KCLOSEST:
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
