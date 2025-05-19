#include <stdio.h>
#include "config.h"
#include "u.h"

extern int Uerror;
extern char *cmdp;
extern int Mrows, Mcols;

void error(), unlist(), skiptok();

/*
 * dolist -- list info about the game.
 */
dolist()
{
	int kflag;	/* store keywords */
	int kword;	/* current keyword */
	int range;	/* range of command */
	STARSHIP *ship;
	int shflag;	/* set if we find any ships */
	int nouns, adjs;

	range = MAX(Mrows, Mcols);
	kflag = 0;
	shflag = 0;
	for (;;) {
		switch(kword = getkword(&ship)) {
		case KTARGETS:	/* targets (enemies in range) */
			kflag |= KENEMY|KNOOUTRNG;
			break;

		case KNEUTRAL:
			kflag |= KPLANETS;
			/* fall through */

		case KCLOSEST:
		case KSHIPS:
		case KFEDERATION:	/* Also KHUMAN */
		case KEMPIRE:		/* Also KKLINGON */
		case KFRIENDLY:
		case KENEMY:
		case KBASES:
		case KPLANETS:
		case KPORTS:	/* ports (both planets and bases) */
		case KCAPTURED:	/* captured planets */
		case KSUMMARY:	/* count -- don't list locations */
			kflag |= kword;
			break;

		case -2:	/* no more tokens */
			/*
			 * IF SHIP SPECIFIED:
			 *  If we specified noun but did not specify
			 *  adjective, take all adjectives.  If we
			 *  specified adjective but did not specify
			 *  noun, take all nouns.
			 * ELSE:
			 *  No adjective means all adjectives.
			 *  No noun means all nouns.
			 */
			nouns = kflag & NOUNS;
			adjs = kflag & ADJS;
			/*
			nouns = kflag & (KSHIPS|KPORTS);
			adjs = kflag & (KALL|KNEUTRAL|KFRIENDLY|KENEMY);
			*/
			if (shflag && (nouns || adjs)) {
				if (nouns && !adjs)
					kflag |= (KALL|KNEUTRAL);
				else if (adjs && !nouns)
					kflag |= NOUNS;
			} else if (!shflag) {
				if (!adjs)
					kflag |= (KALL|KNEUTRAL);
				if (!nouns)
					kflag |= NOUNS;
			}
			return(listthem(kflag, range));

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
		case KALL:	/* both sides */
		case -3:	/* Bad token */
		case -4:	/* semicolon */
		default:
			/* no msgs to send */
			unlist(KSHIPS);
			Uerror = UEBADTOK;
			error("missing semicolon or");
			return(1);
		}
	}
}
