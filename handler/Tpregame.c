#include <stdio.h>
#include <signal.h>
#include "config.h"
#include "uniwar.h"

extern short	mypid;		/* term handler's process id */
extern short	playing;	/* flag TRUE when daemon knows about player */
extern short	cmdstart,	/* first byte to process in cmdbuf buffer */
		cmdend;		/* last byte to process in cmdbuf buffer */
extern int	Mrows,		/* number of rows in galaxy */
		Mcols;		/* number of columns in galaxy */

extern char cmdbuf[];	/* circular buffer for outgoing commands */

void enditall(), moreinit(), polldaemon();

char *gets(), *strcpy(), *strchr();

/*
 * Give user opportunity to examine the game.
 *
 * We're not using windows yet, so use printf() and gets().
 * We have to ignore signals for a bit to close the "window"
 * between sending the first message and setting the "playing"
 * flag to keep from confusing the interrupt handler.
 */
pregame()
{
	char buf[64];
	register int c;
	int (*intsig)();

	/* set up message to daemon asking to begin */
	cmdstart = cmdend = 0;

	/* close window so user hitting BREAK won't confuse daemon. */
	intsig = signal(SIGINT, (int(*)())(SIG_IGN));

	if (sendcmd() < 0)
		return(-1);

	Mrows = Mcols = 0;

	/* wait for reply (either sleeping or can't play) */
	polldaemon(1);
	if (Mrows == 0)	/* didn't get rows and columns on first try */
		polldaemon(1);

	playing = 1;
	(void)signal(SIGINT, intsig);

	/*
	 * now that communications are established, do any initialization
	 * that couldn't be done before.
	 */
	moreinit();

	(void)printf("\tWelcome to the uniwar pregame.\n\n");
	(void)printf("Available commands:\n\n");
	(void)printf("(su)mmary\n(u)sers\n(a)ctivate\n(q)uit\n\n");
	for (;;) {
		(void)printf("PG>");
		(void)gets(buf);
		if (strchr(buf, CMDSEP) != NULL) {
			(void)printf("One command per line in pregame!\n");
			continue;
		}
		switch((int)(*buf)) {
		case 'u':
		case 'U':
		case 's':
		case 'S':
			/* set up message to daemon asking to begin */
			(void)sprintf(cmdbuf, "%s%c", buf, CMDSEP);
			cmdstart = 0;
			cmdend = strlen(cmdbuf);
			if (sendcmd() < 0)
				return(-1);

			/* wait for reply */
			polldaemon(1);

			(void)getmsgch();	/* strip leading UmTEXT */
			while((c = getmsgch()) != '\0')
				(void)putchar(c);
			break;

		case 'a':
		case 'A':
			/* get info from daemon */
			if (startup() < 0)
				return(-1);
			else
				return(0);

		case 'q':
		case 'Q':
			enditall(0);

		default:
			(void)printf("\"%s\" cmd not available in pregame\n"
							, buf);
			break;
		}
	}
	/* only way out of this routine is 'q', 'a', or interrupt */
}

