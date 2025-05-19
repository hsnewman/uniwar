#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern char *cmdp;
extern int Uerror;
extern char currcmd[];

char *strncpy();

void error();

/*
 * doset -- set various player attributes.
 */
doset()
{
	register int i;

	/* get next keyword */
	switch((int)(*cmdp)) {
	case 'i':
		if (getkey("icdef") >= 0) {
			if (isalpha((int)(*cmdp)))
				switch(getabsrel()) {
				case ABSO:
					CurrPl->flags |= ABSOIN;
					return(1);

				case RELA:
					CurrPl->flags &= ~ABSOIN;
					return(1);

				default:
					break;
				}
		}
		break;

	case 'n':
		if (getkey("name") >= 0) {
			(void)strncpy(CurrPl->name, cmdp, MAXPLNAME);
			return(1);
		}
		break;

	case 'o':
		switch((int)(*(cmdp+1))) {
		case 'c':
			if (getkey("ocdef") >= 0) {
				if (isalpha((int)(*cmdp)))
					switch(getabsrel()) {
					case ABSO:
						CurrPl->flags |= ABSOOUT;
						CurrPl->flags &= ~RELOUT;
						return(1);

					case RELA:
						CurrPl->flags |= RELOUT;
						CurrPl->flags &= ~ABSOOUT;
						return(1);

					case BOTH:
						CurrPl->flags |= RELOUT;
						CurrPl->flags |= ABSOOUT;
						return(1);

					default:
						break;
					}
			}
			break;
		case 'u':
			if (getkey("output") >= 0) {
				if (isalpha((int)(*cmdp))) {
					if (getkey("short") >= 0 ||
						getkey("medium") >= 0)
						CurrPl->flags |= OSHORT;
					else if (getkey("long") >= 0)
						CurrPl->flags &= ~OSHORT;
					return(1);
				}
			}
			break;
		case '\0':
		case ' ':
			Uerror = UEAMBIG;
			error(currcmd);	/* complain to current player */
			return(1);
		default:
			break;
		}
		break;

	case 'w':
		if (getkey("warn") >= 0) {
			if (isdigit((int)(*cmdp))) {
				/* quick and dirty */
				switch(i = atoi(cmdp)) {
				case 0:
				case 1:
				case 2:
				case 3:
					CurrPl->flags &=
						~WARNMASK;
					CurrPl->flags |=
						(i << 11);
					return(1);
				default:
					break;
				}
			}
		}
		break;

	default:
		break;
	}

	Uerror = UEBADTOK;
	error(currcmd);	/* complain to current player */
	return(1);
}
