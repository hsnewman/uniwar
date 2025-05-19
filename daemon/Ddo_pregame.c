#include "config.h"
#include "u.h"

extern STARSHIP fleet[];
extern PLAYER *CurrPl;
extern char currcmd[];
extern char *cmdp;
extern int Uerror;
extern struct stats stats;
extern short npg;	/* number of players in pregame */
extern char Outbuf[];

void error();

/*
 * do_pregame -- prompt player based on his/her flags.
 * We either give the player pregame info, or
 * try to give the player a side and ship.
 */
do_pregame()
{
	register char *cp;
	register STARSHIP *tship;
	register short i;

	cp = Outbuf+sizeof(short);

	/* player is in pregame */
	if (CurrPl->flags == 0) {
		switch(gettok()) {
		case ACTIVATE:
			*cp++ = UmINIT;
			CurrPl->flags |= GETSIDE;
pickside:
			(void)sprintf(cp, "Currently there are %d empire ships and",
				stats.Ecnt);
			while(*cp != '\0') cp++;
			(void)sprintf(cp, " %d federation ships.\n", stats.Fcnt);
			while(*cp != '\0') cp++;
			(void)sprintf(cp, "Which side will you join (E or F): ");
			return(sendcmd(CurrPl, strlen(Outbuf+sizeof(short))+1));

		case SUMMARY:
			return(dosumm());

		case USERS:
			return(dousers());

		default:
			/* command not available in pregame */
			Uerror = UENOTPG;
			error(currcmd);
			return(1);
		}
	}
	*cp++ = UmINIT;
	if (CurrPl->flags & GETSIDE) {
		tship = fleet;
		if (currcmd[0] == 'e') {
			CurrPl->flags |= EMPIRE;
			tship += MAXPL/2;
		} else if (currcmd[0] == 'f')
			CurrPl->flags |= FEDDIE;
		else {
			(void)sprintf(cp, "No such alliance.\n\n");
			while(*cp != '\0') cp++;
			goto pickside;
		}
		CurrPl->flags &= ~GETSIDE;
		CurrPl->flags |= GETSHIP;
pickship:
		(void)sprintf(cp, "Available ships:\n\n");
		while (*cp != '\0') cp++;
		for (i = 0; i < MAXPL/2; i++) {
			if ((tship[i].flags & ACTIVE) == 0)
				(void)sprintf(cp, "%s\n", tship[i].name);
			while(*cp != '\0') cp++;
		}
		(void)sprintf(cp, "\nWhich ship do you want: ");
		return(sendcmd(CurrPl, strlen(Outbuf+sizeof(short))+1));
	}
	/* If we got to here, CurrPl->flags must be set to GETSHIP */
	/* check for legal ship -- if not legal, go to pickship */
	tship = fleet;
	if (CurrPl->flags & EMPIRE)
		tship += MAXPL/2;
	for (i = 0; i < MAXPL/2; i++) {
		if (tship[i].name[0] == toupper(currcmd[0]))
			break;
		if ((tship[i].flags & ACTIVE) == 0 &&
			tship[i].name[0] == toupper(currcmd[0]))
			break;	/* found a ship */
	}
	if (i == MAXPL/2) {
		(void)sprintf(cp, "%s is not a%s ship.\n\n", currcmd
			, (tship == fleet) ? " Federation" : "n Empire");
		while(*cp != '\0') cp++;
			goto pickship;
	}
	if (tship[i].flags & ACTIVE) {
		(void)sprintf(cp, "Sorry, the %s is in use.\n", tship[i].name);
		while(*cp != '\0') cp++;
		goto pickship;
	}
	CurrPl->ship = &tship[i];
	npg--;	/* one less person in pregame */
	CurrPl->ship->owner = CurrPl;
	CurrPl->ship->flags |= ACTIVE;
	CurrPl->flags &= ~GETSHIP;

	/*
	 * activate player, raise shields, turn on radio,
	 * set both absolute and relative coords on output,
	 * enable scans.
	 */
	CurrPl->flags |= ACTIVE|SHIELDED|RADIO|ABSOOUT|RELOUT|SCANON;
	CurrPl->flags |= WARN2;	/* put box around danger areas */
	CurrPl->baudrate = 9600;
	CurrPl->phtime1 = CurrPl->phtime2 = CurrPl->totime = 0;
	CurrPl->pending = 0;
	CurrPl->condition = GREEN;
	CurrPl->lifersrv = MAXLIFE;
	CurrPl->energy = MAXNRGY;
	CurrPl->torps = MAXTORP;
	CurrPl->shields = MAXSHIE;
	CurrPl->damage = 0;
	CurrPl->stardate = 0;

	for (i = 0; i < NSCORE; i++)
		CurrPl->score[i] = 0;	/* no points yet */

	for (i = 0; i < NDEVS; i++)
		CurrPl->damdev[i] = 0;	/* no damage to devices */

	/* put him in the universe and send him a scan and status report */
	(void)activate();
	return(1);
}
