#include "config.h"
#include "u.h"

extern struct devinfo devinfo[];

char *telldmg();
char *strcat();
void needscan();

/*
 * hitdevs -- hit pl in the devices with "hit".
 * Tell pl the gory details.
 */
hitdevs(hit, pl)
long hit;
PLAYER *pl;
{
	long chunk, mchunk;
	short wham;
	short newdam[NDEVS];
	char buf[1024];
	int i, t;

	*buf = '\0';
	for (i = 0; i < NDEVS; i++)
		newdam[i] = 0;

	/*
	 * there's a chance in 20 that the whole hit is going to a single
	 * device!  That should be interesting.
	 */
	if ((rnd() % 20) == 0)
		wham = 1;
	else
		wham = 0;
	mchunk = hit/5;
	while (hit) {
		if (wham)
			chunk = hit;
		else
			chunk = MIN(rndrange(100,(int)MAX(10000,mchunk)),hit);

		/* choose a device */
		t = rndrange(0, 99);
		for (i = 0; t >= devinfo[i].hitprob; i++)
			;
		if (pl->damdev[i] > devinfo[i].maxdmg*3)
			continue;	/* destroyed already */

		/* damage on shields knocks them down 25% of the time */
		if (i == DSHIELD && (pl->flags & SHIELDED) && !(rnd() % 4)) {
			pl->flags &= ~SHIELDED;
			needscan(pl->rpos, pl->cpos);
		}
		newdam[i]++;
		pl->damdev[i] += chunk;
		hit -= chunk;
	}

	/* tell player about the device that was hit hardest */
	t = 0;
	for (i = 1; i < NDEVS; i++) {
		if (newdam[i] > newdam[t])
			t = i;
	}
	(void)strcpy(buf, telldmg(t, pl));
	if (pmsg(UmTEXT, pl, buf, -1) < 0)
		return(-1);
	return(1);
}
