#include "config.h"
#include "u.h"

extern struct devinfo devinfo[];
extern struct stats stats;

char *strcat();

/*
 * send pl's ship status to pl.
 */
dostatus(pl)
PLAYER *pl;
{
	char tbuf[512];
	char lbuf[80];
	char *tmp;

	*tbuf = '\0';

	/* condition and radio */
	switch((int)pl->condition) {
	case RED:
		tmp = "Red   ";
		break;

	case YELLOW:
		tmp = "Yellow";
		break;

	case GREEN:
		tmp = "Green ";
		break;

	default:
		tmp = "Weird ";
		break;
	}
	(void)sprintf(tbuf, "Cond:%s Scan:%s Ships:%2d\n", tmp
				, (pl->flags & SCANON) ? "ON " : "OFF"
					, stats.Ecnt + stats.Fcnt);

	/* energy and shields */
	(void)sprintf(lbuf, "Energy:  %4d.%02d (%3d%%)\n", pl->energy/100
		, pl->energy % 100, (pl->energy * 100)/MAXNRGY);
	(void)strcat(tbuf, lbuf);
	(void)sprintf(lbuf, "Shields: %4d.%02d (%c%d%%)  \n", pl->shields/100
		, pl->shields % 100, (pl->flags & SHIELDED) ? '+' : '-'
		, (pl->shields * 100)/MAXSHIE);
	(void)strcat(tbuf, lbuf);

	/* torpedoes and ship damage */
	(void)sprintf(lbuf, "torps: %2d  damage: %4d.%02d\n", pl->torps
		, pl->damage/100, pl->damage % 100);
	(void)strcat(tbuf, lbuf);

	/* stardate (moves to date) */
	(void)sprintf(lbuf, "stardate %d", pl->stardate);
	(void)strcat(tbuf, lbuf);

	/* life support critically damaged? */
	if (pl->damdev[DLIFE] > devinfo[DLIFE].maxdmg)
		(void)sprintf(lbuf, " *LIFE SUPPORT* %hd", pl->lifersrv);
	else
		(void)sprintf(lbuf, "   score:%6d  ", pl->score[STOTAL]);
	(void)strcat(tbuf, lbuf);

	/* send the message */
	if (pmsg(UmSTATUS, pl, tbuf, -1) < 0)
		return(-1);
	return(1);
}
