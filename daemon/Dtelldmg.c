#include "config.h"
#include "u.h"

extern struct devinfo devinfo[];

char *strcpy(), *strcat();

/*
 * telldmg -- tell the player about his damaged devices.
 * if dev is -1, tell about all of them.
 */
char *
telldmg(dev, pl)
int dev;
PLAYER *pl;
{
	static char buf[MAXMSG];
	char dmg[64];
	register int first, last, i;

	if (dev == -1) {
		first = 0;
		last = NDEVS;
	}
	else {
		first = dev;
		last = dev+1;
	}
	*buf = '\0';
	for (i = first; i < last; i++) {
		if (pl->damdev[i] > devinfo[i].maxdmg*3)
			(void)strcpy(dmg, "destroyed!");
		else if (pl->damdev[i] > devinfo[i].maxdmg)
			(void)sprintf(dmg, "crippled (%d)!", pl->damdev[i]/100);
		else if (pl->damdev[i] > 0)
			(void)sprintf(dmg, "damaged %d", pl->damdev[i]/100);
		else
			continue;
		(void)sprintf(&buf[strlen(buf)], "%s %s\n", devinfo[i].name
								, dmg);
	}
	if (strlen(buf) == 0)
		(void)strcpy(buf, "No damage to devices\n");

	/* if we're listing them all, append an extra newline */
	if (dev == -1)
		(void)strcat(buf, "\n");
	return(buf);
}
