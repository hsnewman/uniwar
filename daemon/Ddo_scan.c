#include <stdio.h>
#include "config.h"
#include "u.h"

extern char *cmdp;
extern PLAYER *CurrPl;
extern int Uerror;

void error();

/*
 * do_scan - turn scanner on and off:
 *	scan on		-- do scans
 *	scan off	-- don't do scans
 */
do_scan()
{
	register char *savcmdp;

	if (*cmdp == 'o') {
		savcmdp = cmdp;
		if (getkey("on") >= 0) {	/* scanners on */
			CurrPl->flags |= SCANON;
			return(pmsg(UmTEXT, CurrPl, "Scanners enabled\n", -1));
		} else {
			cmdp = savcmdp;
			if (getkey("off") >= 0) {	/* scanners off */
				CurrPl->flags &= ~SCANON;
				return(pmsg(UmTEXT, CurrPl
						, "Scanners disabled\n", -1));
			}
		}
	} else if (*cmdp != '\0') {
		Uerror = UEBADTOK;
		error(cmdp);
	}
	return(1);
}
