#include "config.h"
#include "uniwar.h"

extern int Uerror;

void enditall(), play();

/*
 * This is the uniwar terminal handler (should be run in the
 * foreground by each player).
 */
main(ac, av)
int ac;
char *av[];
{
	/*
	 * See if daemon FIFO is there and create our own FIFO.
	 * Read environment variable UNIWAROPTS for initial commands.
	 */
	if (init(ac, av) == -1)
		enditall(0);
	
	/*
	 * Pregame -- allow user to see what's happening.
	 */
	if (pregame() == -1)
		enditall(0);
	
	/*
	 * Catch signals and initialize curses stuff.
	 */
	if (cursinit() == -1)
		enditall(0);
	
	/*
	 * Play until dead or quit.  Don't care if there was an error
	 * because we're going to exit next anyway.
	 */
	play();
	enditall(0);	/* all done */
	return(0);	/* to make lint happy */
}

