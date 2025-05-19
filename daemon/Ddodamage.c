#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;

char *telldmg();

/*
 * dodamage -- give a damage report
 */
dodamage()
{
	if (pmsg(UmTEXT, CurrPl, telldmg(-1, CurrPl), -1) < 0)
		return(-1);
	return(1);
}
