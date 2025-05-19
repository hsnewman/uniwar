#include <stdio.h>
#include "config.h"
#include "u.h"

extern char *cmdp;
extern int Uerror;
extern PLAYER *CurrPl;

void error();

/*
 * dorepair -- do repairs on devices.  One optional argument, 
 * "units to repair" (default is DEFREP).  Takes PREPAIR seconds
 * to repair 10 units of damage.
 *
 * If docked, repair is twice as fast as normal, and default
 * repair job is doubled.
 */
dorepair()
{
	int units;
	char buf[4];
	int repwait;

	if (*cmdp) {	/* there's an arg there */
		switch(sscanf(cmdp, "%d %1s", &units, buf)) {
		case EOF:
		case 0:
			Uerror = UENONNUM;
			error("");
			return(1);
		case 1:
			break;
		
		default:
			Uerror = UEBIGCMD;
			error("");
			return(1);
		}
		if (units <= 0) {
			Uerror = UEQUAN;
			error("");
			return(1);
		}
		units *= 100;
	} else {
		units = DEFREP;
		if (CurrPl->flags & DOCKED)
			units *= 2;
	}

	/* actual amount repaired */
	if ((units = repair(units)) == 0)
		return(1);

	repwait = MAX((units/2500) * PREPAIR, 1);
	if (CurrPl->flags & DOCKED)
		repwait = MAX(repwait/2, 1);

	if (psleep(CurrPl, repwait) < 0)
		return(-1);
	return(1);
}
