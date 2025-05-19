#include <stdio.h>
#include <signal.h>
#include "config.h"
#include "u.h"

extern short usedship;
extern PLAYER ships[];
extern PLAYER *CurrPl;
extern PLANET *planets;
extern struct stats stats;
extern int nocleanup;	/* set if cleanup should not be done on interrupt */

void free();

/*
 * Send the last message to the players.
 */
cleanup()
{
	register int i;
	char *msg, *emsg, *fmsg;
	char ports_remain[64];

	/* declare the winner in cleanup */
	sprintf(ports_remain, "There %s %d port%sremaining.\n",
		((stats.nport > 1)?"were":"was"), stats.nport,
		((stats.nport > 1)?"s ":" "));
	if (stats.nbase[EMPIRE] + stats.nplan[EMPIRE] == stats.nport) {
		emsg =
"The Klingon Empire has emerged victorious!\n\
Begin slave operations immediately.\n";
		fmsg =
"The Klingon Empire has emerged victorious!\n\
Please proceed to the nearest Klingon slave planet.\n";
	} else {
		emsg =
"The Federation has emerged victorious!\n\
Initiate self-destruct sequence.\n";
		fmsg =
"The Federation has emerged victorious!\n\
Freedom again rules the galaxy.\n";
	}
#ifdef DEBUG
printf("%s%s", emsg, fmsg);
#endif
	for (i = usedship; i != -1; i = CurrPl->list) {
		CurrPl = &ships[i];
#ifdef DEBUG
printf("ship %d being handled\n", i);
#endif
		if ((CurrPl->flags & DIED) || (CurrPl->flags & ACTIVE) == 0)
			continue;
		if (pmsg(UmTEXT, CurrPl, "THE WAR IS OVER!!\n", -1) < 0)
			return(-1);
		if (CurrPl->flags & FEDDIE)
			msg = fmsg;
		else
			msg = emsg;
		if (pmsg(UmTEXT, CurrPl, msg, -1) < 0)
			return(-1);
		if (pmsg(UmTEXT, CurrPl, ports_remain, -1) < 0)
			return(-1);
#ifdef DEBUG
printf("ship %d handled\n", i);
#endif
	}

	/* send stats to all players and kill them */
	if (dopoint(&ships[usedship], 1) < 0)
		return(-1);
#ifdef DEBUG
printf("returned from dopoint()\n", i);
#endif

	nocleanup = 1;	/* no cleanup on interrupt, just quit */

	if (planets != NULL) {
		free((char *)planets);
		planets = (PLANET *)NULL;
	}
	return(waitforgoodbyes());
}
