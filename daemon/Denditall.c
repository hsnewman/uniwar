#include <signal.h>
#include "config.h"
#include "u.h"

extern int nocleanup;	/* set if cleanup should not be done on interrupt */
extern char Ififonam[];

void exit();
unsigned sleep();

/*
 * Close up shop before exiting.
 * Send last messages to all players, wait 3 seconds, then
 * destroy input FIFO.
 */
void
enditall(sig)
int sig;
{
	if (sig != 0)
		(void)signal(sig, SIG_IGN);

	if (!nocleanup) {
		(void)cleanup();	/* if error, what can I do? */
		(void)sleep(3);		/* may need time for word to get out */
	}

	/* remove FIFO file */
	(void)unlink(Ififonam);
	exit(0);
}
