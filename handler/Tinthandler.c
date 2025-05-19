#include <curses.h>	/* curses.h automatically includes stdio.h */
#include <signal.h>
#include "config.h"
#include "uniwar.h"

extern WINDOW	*textwin;	/* window containing text messages */

extern int	Uerror;		/* stores uniwar error type for error() */
extern short	playing;	/* flag TRUE when daemon knows about player */
extern short	cmdstart,	/* first byte to process in cmdbuf circular buffer */
		cmdend;		/* last byte to process in cmdbuf circular buffer */
extern char	Outbuf[];	/* buffer for outgoing commands */
extern int	interrupted;

void enditall();
char *memcpy();

/*
 * Hitting interrupt should abort stacked commands.
 */
void
inthandler(sig)
register int sig;
{
	char tmpbuf[8];

	(void)signal(sig, (int(*)())(SIG_IGN));
	if (sig == SIGINT)
		interrupted = 1;

	/* may do other stuff here later... */

	if (!playing) {
		/* wants out before s/he even starts */
		Uerror = UEINTR;
		enditall(sig);
	}
	/* we're playing if we get to here */
	if (textwin != NULL && sig == SIGINT) {
		/* he doesn't want out, he wants to abort stacked comands */
		cmdstart = cmdend = 0;	/* throw out outgoing info */

		/* may have been using Outbuf -- save front */
		(void)memcpy(tmpbuf, Outbuf, 8);
		Outbuf[2*sizeof(short)] = ABORT; /* probably ^C */
		(void)write_it((short)1);
		(void)memcpy(Outbuf, tmpbuf, 8);
	} else {	/* he typed SIGQUIT or subwindows not up yet */
		Uerror = UEINTR;
		enditall(sig);
	}

	(void)signal(sig, inthandler);
}
