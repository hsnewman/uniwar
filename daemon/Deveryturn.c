#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern struct devinfo devinfo[];
extern long allstardate[];

/*
 * everyturn -- do things we do after every move.
 * repflg is set after a "significant action" such
 * as a move, dock, capture, or build.
 * When it's set, there's time for some repairs
 * to be done.  Stardate is bumped after any command.
 * Status report is always sent.
 */
everyturn(repflg)
int repflg;
{
	if (CurrPl->damdev[DLIFE] > devinfo[DLIFE].maxdmg) {
		CurrPl->lifersrv--;
		if (CurrPl->lifersrv == 0)
			return(saybyebye(CurrPl));
		if (CurrPl->flags & DOCKED)
			CurrPl->lifersrv = MAXLIFE;
	}
	if (repflg) {
		if (CurrPl->flags & DOCKED)
			(void)repair(TURNREP*2);
		else
			(void)repair(TURNREP);
	}

	CurrPl->stardate++;
	allstardate[CurrPl->flags & ALIMASK]++;

	if (dostatus(CurrPl) < 0)
		return(-1);
	return(1);
}
