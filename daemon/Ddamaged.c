#include "config.h"
#include "u.h"

extern struct devinfo devinfo[];

/*
 * damaged -- return damage in *dam.  Return 0 if device operational.
 * Return 1 if device inoperational.
 */
damaged(dev, pl, dam)
int dev;
PLAYER *pl;
long *dam;
{
	*dam = pl->damdev[dev];
	if (*dam > devinfo[dev].maxdmg) {
		return(1);
	} else
		return(0);
}

/*
 * critmsg -- tell pl his device is critically damaged.
 */
critmsg(pl, dev)
PLAYER *pl;
int dev;
{
	char buf[128];

	(void)sprintf(buf, "%s critically damaged (%d units)\n"
			, devinfo[dev].name, pl->damdev[dev]/100);
	return(pmsg(UmTEXT, pl, buf, -1));
}
