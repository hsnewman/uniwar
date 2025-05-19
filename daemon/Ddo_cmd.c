#include "config.h"
#include "u.h"

extern char currcmd[];
extern PLAYER *CurrPl;
extern int Uerror;

void error();

/*
 * do_cmd -- call the correct routine to handle the command in
 * currcmd[].
 * return -1 on fatal error, 0 if game ended, 1 if game continues.
 *
 * Whenever the map changes in range of a player, rescan for that player.
 * For now, scan after every successful command.  Scan calls hotspot(),
 * so any command by any player might get baddies firing at scanner.
 *
 * repflg is set if repairs are to be done after the command.
 * hotflg is set if we give enemies ports a shot at us after the command.
 */
do_cmd()
{
	register int ret;
	register int tok;
	int repflg, hotflg;
	int happen;	/* set if time-consuming command */

	/* if trying to pick a side or ship */
	if ((CurrPl->flags & ACTIVE) == 0)
		return(do_pregame());

	repflg = 0;	/* default to not doing repairs */
	hotflg = 0;	/* default to no potshots */
	happen = 0;	/* default to not-time-consuming */
	switch(tok = gettok()) {
	case MOVE:
	case IMPULSE:
		ret = domove(tok);
		repflg = 1;
		hotflg = 1;
		happen = 1;
		break;
	
	case CAPTURE:
		ret = dobuild(tok);
		repflg = 1;
		hotflg = 1;
		happen = 1;
		break;

	case BUILD:
		ret = dobuild(tok);
		repflg = 1;
		hotflg = 1;
		happen = 1;
		break;

	case PHASER:
		ret = dophaser();
		hotflg = 1;
		happen = 1;
		break;

	case SET:
		ret = doset();
		break;

	case TELL:
		ret = dotell();
		break;

	case LIST:
		ret = dolist();
		break;

	case BASES:
		ret = dobapl(KBASES);
		break;

	case PLANETS:
		ret = dobapl(KPLANETS);
		break;

	case TARGETS:
		ret = dotarg();
		break;

	case SUMMARY:
		ret = dosumm();
		break;

	case QUIT:
		ret = doquit();
		break;

	case SHIELD:
		ret = doshield();
		break;

	case SCAN:
		ret = do_scan();	/* may turn scan on or off */
		break;

	case DOCK:
		ret = dodock();
		repflg = 1;
		hotflg = 1;
		happen = 1;
		break;

	case DAMAGE:
		ret = dodamage();
		break;

	case REPAIR:
		ret = dorepair();
		hotflg = 1;
		happen = 1;
		break;

	case USERS:
		ret = dousers();
		break;

	case TORPEDO:
		ret = dotorp();
		hotflg = 1;
		happen = 1;
		break;

	case POINTS:
		ret = dopoint(CurrPl, 0);
		break;

	/* we'll allow other commands here later */

	case AMBIGTOK:
		Uerror = UEAMBIG;
		error(currcmd);	/* complain to current player */
		ret = 1;
		break;

	case ACTIVATE:
	case BADTOK:
		Uerror = UEBADTOK;
		error(currcmd);	/* complain to current player */
		ret = 1;
		break;

	default:
		Uerror = UEINTL;
		ret = -1;
		break;
	}
	if (tok != QUIT)
		CurrPl->flags &= ~WANTSOUT;

	if (ret == 1) {
		if (hotflg)
			(void)hotspot(CurrPl, 1);
		ret = everyturn(repflg);
	}

	if (ret == 1) {
		if (allscan() < 0)
			return(-1);
		if (happen)	/* time-consuming cmd -- return '1' */
			return(ret);
		else
			ret++;	/* not a time-consuming command
					so return '2' */
	}

	return(ret);
}
