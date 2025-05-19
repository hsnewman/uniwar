#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern char *cmdp;

/*
 * Current player wants to quit.
 * If he's on red alert, he can't quit
 * He may have quit accidentally -- make him enter QUIT again
 * unless he typed q !
 */
doquit()
{
	if (CurrPl->condition == RED) {
		if (pmsg(UmTEXT, CurrPl, "Can't quit on condition RED\n"
							, -1) < 0)
			return(-1);
		return(1);
	}

	if ((CurrPl->flags & WANTSOUT) || *cmdp == '!')
		return(saybyebye(CurrPl));
	CurrPl->flags |= WANTSOUT;
	if (pmsg(UmTEXT, CurrPl, "Quit again if you really\nwant to quit.\n"
							, -1) < 0)
		return(-1);
	return(1);
}
