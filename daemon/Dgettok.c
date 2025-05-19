#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "u.h"

extern char *cmdp;
extern int Uerror;

STARSHIP *getship();

/*
 * gettok returns the symbolic constant associated with the first
 * token in cmdp, setting cmdp to point to the next token.
 * It assumes that cmdp is null terminated, contains exactly
 * one blank after each token (except the last), and is mapped
 * to lower case.  All that work should have been done by
 * cleancmd().
 */
gettok()
{
	switch((int)(*cmdp)) {
	case 'a':
		if (getkey("activate") >= 0)
			return(ACTIVATE);
		break;

	case 'b':
		switch((int)(*(cmdp+1))) {
		case 'a':
			if (getkey("bases") >= 0)
				return(BASES);
			break;
		case 'u':
			if (getkey("build") >= 0)
				return(BUILD);
			break;
		case '\0':
		case ' ':
			return(AMBIGTOK);
		default:
			break;
		}
		break;

	case 'c':
		if (getkey("capture") >= 0)
			return(CAPTURE);
		break;

	case 'd':
		switch((int)(*(cmdp+1))) {
		case 'a':
			if (getkey("damage") >= 0)
				return(DAMAGE);
			break;
		case 'o':
			if (getkey("dock") >= 0)
				return(DOCK);
			break;
		case '\0':
		case ' ':
			return(AMBIGTOK);
		default:
			break;
		}
		break;

	case 'i':
		if (getkey("impulse") >= 0)
			return(IMPULSE);
		break;

	case 'l':
		if (getkey("list") >= 0)
			return(LIST);
		break;

	case 'm':
		if (getkey("move") >= 0)
			return(MOVE);
		break;

	case 'p':
		switch((int)(*(cmdp+1))) {
		case 'h':
			if (getkey("phaser") >= 0)
				return(PHASER);
			break;
		case 'l':
			if (getkey("planets") >= 0)
				return(PLANETS);
			break;
		case 'o':
			if (getkey("points") >= 0)
				return(POINTS);
			break;
		case '\0':
		case ' ':
			return(AMBIGTOK);
		default:
			break;
		}
		break;

	case 'q':
		if (getkey("quit") >= 0)
			return(QUIT);
		break;

	case 'r':
		if (getkey("repair") >= 0)
			return(REPAIR);
		break;
	
	case 's':
		switch((int)cmdp[1]) {
		case 'c':
		case 'r':
			if (getkey("scan") >= 0 || getkey("sr") >= 0)
				return(SCAN);
			break;
		case 'e':
			if (getkey("set") >= 0)
				return(SET);
			break;
		case 'h':
			if (getkey("shields") >= 0)
				return(SHIELD);
			break;
		case 'u':
			if (getkey("summary") >= 0)
				return(SUMMARY);
			break;
		/***
		case 't':	** invalid command -- status always done **
			if (getkey("status") >= 0)
				return(STATUS);
			break;
		***/
		case '\0':
		case ' ':
			return(AMBIGTOK);
		default:
			break;
		}
		break;

	case 't':
		switch((int)cmdp[1]) {
		case 'a':
			if (getkey("targets") >= 0)
				return(TARGETS);
			break;
		case 'o':
			if (getkey("torpedo") >= 0)
				return(TORPEDO);
			break;
		case 'e':
			if (getkey("tell") >= 0)
				return(TELL);
			break;
		case '\0':
		case ' ':
			return(AMBIGTOK);
		default:
			break;
		}
		break;

	case 'u':
		if (getkey("users") >= 0)
			return(USERS);
		break;
	
	default:
		break;
	}
	return(BADTOK);
}

/*
 * skiptok -- cmdp points to a token; move cmdp to the next token
 */
void
skiptok()
{
	while (!isspace((int)(*cmdp)) && *cmdp)
		++cmdp;
	if (*cmdp)
		++cmdp;	/* should be exactly one space there */
}

/*
 * abs_rel expects cmdp to point to string of tokens.
 * First token is [Absolute|Relative|Computed].
 * If the first token is Absolute or Relative, the
 * next two tokens must be integers.
 * If the first token is Computed, the next token
 * must be a ship, or a number followed by a ship.
 * If first token is a number, expect two numbers and
 * handle them based on (pl->flags & ABSOIN)
 *
 * Return relative row and col increments required for
 * player pl to get to required spot.
 */
abs_rel(pl, r, c, ship, size)
PLAYER *pl;
short *r, *c;
STARSHIP **ship;
long *size;
{
	short howmove;
	char buf[8];

	if (isalpha((int)(*cmdp))) {
		switch((int)(howmove = (short)getabsrel())) {
		case ABSO:
		case RELA:
		case COMP:
			break;

		default:
			Uerror = UEBADTOK;
			return(-1);
		}
	} else if (pl->flags & ABSOIN)
		howmove = ABSO;
	else
		howmove = RELA;

	*size = -1;
	/* if size is parsed, make sure it's positive */
	if (isdigit((int)(*cmdp))) {
		/*
		 * parse phaser shot size or number of torps --
		 * may turn out to be row coord
		 */
		(void)sscanf(cmdp, "%ld", size);
		skiptok();
	}

	*ship = (STARSHIP *)NULL;
	if (howmove == COMP) {

		/* get ship from arglist starting at cmdp */
		Uerror = 0;
		if ((*ship = getship()) == (STARSHIP *)NULL)
			Uerror = UEBADSHIP;
		else if (*cmdp != '\0')
			Uerror = UEBIGCMD;
		else if (((*ship)->flags & ACTIVE) == 0)
			Uerror = UEISEMPTY;
		else if (((*ship)->owner->flags & DIED) == 1)
			Uerror = UEISDEAD;
		if (Uerror != 0)
			return(-1);
		*r = (*ship)->owner->rpos - pl->rpos;
		*c = (*ship)->owner->cpos - pl->cpos;
		return(1);
	}

	/* Now we expect there to be two more arguments, both numeric */
	switch(sscanf(cmdp, "%hd %hd %s", r, c, buf)) {
	case 1:
		if (*size >= 0) { /* wasn't a size after all, was a coord */
			*c = *r;
			*r = *size;
			*size = -1;
			break;
		}
		/* fall through */

	case EOF:
	case 0:
		Uerror = UENONNUM; /* non-numeric or missing args */
		return(-1);
	
	case 2:
		break;

	default:
		Uerror = UEBIGCMD;
		return(-1);
	}
	if (howmove == ABSO) {	/* convert to relative */
		*r = *r - pl->rpos;
		*c = *c - pl->cpos;
	}
	return(1);
}

/*
 * getabsrel -- expect next token in cmdp to be
 * Absolute|Relative|Computed|Both.  For use in abs_rel()
 * and "set icdef" and "set ocdef".
 */
getabsrel()
{
	switch((int)(*cmdp)) {
	case 'a':
		if (getkey("absolute") >= 0)
			return(ABSO);

	case 'r':
		if (getkey("relative") >= 0)
			return(RELA);

	case 'c':
		if (getkey("computed") >= 0)
			return(COMP);

	case 'b':
		if (getkey("both") >= 0)
			return(BOTH);

	default:
		Uerror = UEBADTOK;
		return(-1);
	}
}

/*
 * getkword -- get a keyword, return the symbolic constant
 * associated with it.  Some extra tests are done to help
 * make ship name match -- e.g., 'n' should be Nimitz, not
 * Neutral.
 *
 * Return -1 if next token is numeric.
 * Return -2 if there are no more tokens.
 * Return -3 if token can't be interpreted (Ambiguous or Bad Token).
 * Return -4 if semicolon is found (msg probably follows).
 * Return -5 if a ship is found, and return a pointer to
 *	the ship in *ship.
 */
getkword(ship)
STARSHIP **ship;
{
	char *savcmdp;

	savcmdp = cmdp;
	*ship = (STARSHIP *)NULL;
	switch((int)(*cmdp)) {
	case 'a':
		if (cmdp[1] == 'l' && getkey("all") >= 0)
			return(KALL);
		break;

	case 'b':
		if (cmdp[1] == 'a' && getkey("bases") >= 0)
			return(KBASES);
		break;

	case 'c':
		switch((int)cmdp[1]) {
		case 'a':
			if (getkey("captured") >= 0)
				return(KCAPTURED);
			break;
		case 'l':
			if (getkey("closest") >= 0)
				return(KCLOSEST);
			break;
		case '\0':
		case ' ':
		default:
			break;
		}
		break;

	case 'e':
		switch((int)cmdp[1]) {
		case 'm':
			if (getkey("empire") >= 0)
				return(KEMPIRE);
			break;
		case 'n':
			if (getkey("enemy") >= 0)
				return(KENEMY);
			break;
		case '\0':
		case ' ':
		default:
			break;
		}
		break;

	case 'f':
		switch((int)cmdp[1]) {
		case 'e':
			if (getkey("federation") >= 0)
				return(KFEDERATION);
			break;
		case 'r':
			if (getkey("friendly") >= 0)
				return(KFRIENDLY);
			break;
		case '\0':
		case ' ':
		default:
			break;
		}
		break;

	case 'h':
		if (cmdp[1] == 'u' && getkey("human") >= 0)
			return(KHUMAN);
		break;

	case 'k':
		if (cmdp[1] == 'l' && getkey("klingon") >= 0)
			return(KKLINGON);
		break;

	case 'n':
		if (cmdp[1] == 'e' && getkey("neutral") >= 0)
			return(KNEUTRAL);	/* neutral */
		break;

	case 'p':
		switch((int)cmdp[1]) {
		case 'l':
			if (getkey("planets") >= 0)
				return(KPLANETS);
			break;
		case 'o':
			if (getkey("ports") >= 0)
				return(KPORTS);
			break;
		case '\0':
		case ' ':
		default:
			break;
		}
		break;

	case 's':
		switch((int)cmdp[1]) {
		case 'h':
			if (getkey("ships") >= 0)
				return(KSHIPS);
			break;
		case 'u':
			if (getkey("summary") >= 0)
				return(KSUMMARY);
			break;
		case '\0':
		case ' ':
		default:
			break;
		}
		break;

	case 't':
		if (cmdp[1] == 'a' && getkey("targets") >= 0)
			return(KTARGETS);
		break;

	case '\0':
		return(-2);	/* no more tokens */

	case ';':
		skiptok();
		return(-4);

	case '-':
		if (isdigit((int)cmdp[1]))
			return(-1);	/* numeric (negative) */
	default:
		break;
	}

	/* We either got a bad or ambiguous token or a ship name. */
	cmdp = savcmdp;

	if (isdigit((int)cmdp[0]))
		return(-1);	/* numeric (non-negative) */
	if (*ship = getship()) {	/* got one! */
		Uerror = 0;
		if (((*ship)->flags & ACTIVE) == 0)
			Uerror = UEISEMPTY;
		else if (((*ship)->owner->flags & DIED) == 1)
			Uerror = UEISDEAD;
		return(-5);
	}
	return(-3);
}
