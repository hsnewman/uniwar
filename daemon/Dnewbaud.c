#include "config.h"
#include "u.h"

extern int baudincr;
extern struct stats stats;

/*
 * newbaud -- we have a new baud rate 'baud' for player pl.
 * change his rate after adjusting stats structure and
 * baudincr.
 */
void
newbaud(pl, baud)
register PLAYER *pl;
register int baud;
{
	register int sum;

	/* add new baud rate */
	if (baud < 1200)
		stats.baudcnt[BAUD300]++;
	else if (baud < 4800)
		stats.baudcnt[BAUD1200]++;

	/*
	 * adjust baudincr by the following rules:
	 * Set baudincr based on slowest terminal in game.
	 * If everyone is the same speed, baudincr == 0.
	 */
	sum = 0;
	if ((sum = stats.baudcnt[BAUD300]) > 0)
		baudincr = 2;
	else if ((sum = stats.baudcnt[BAUD1200]) > 0)
		baudincr = 1;
	else
		baudincr = 0;

	if (sum == stats.Ecnt + stats.Fcnt)
		baudincr = 0;
	pl->baudrate = baud;
#ifdef DEBUG
printf("new baudrate %d, sum=%d, Ecnt=%d, Fcnt=%d\n",
	baud, sum, stats.Ecnt, stats.Fcnt);
#endif
}
