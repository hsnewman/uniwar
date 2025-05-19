#include <pwd.h>
#include <stdio.h>
#include "config.h"
#include "uniwar.h"

extern int	Uerror;		/* stores uniwar error type for error() */
extern short	cmdstart,	/* first byte to process in cmdbuf buffer */
		cmdend;		/* last byte to process in cmdbuf buffer */
extern short	msgstart;	/* first byte to process in msgbuf buffer */
extern short	active;		/* player is activated in daemon's eyes */

extern char cmdbuf[];	/* circular buffer for outgoing commands */
extern char msgbuf[];	/* circular buffer for incoming messages */

void polldaemon();

void setpwent(), endpwent();
struct passwd *getpwuid();
unsigned short getuid();

char *strcat(), *strcpy(), *getenv();

/*
 * Talk to the daemon -- pass UNIWAROPTS cmds, pick a ship.
 */
startup()
{
	register char *s;
	register int c;
	struct passwd *passwd;

	/* set up message to daemon asking to begin */
	(void)strcpy(cmdbuf, "a/");
	cmdstart = 0;
	cmdend = 2;
	for (;;) {	/* get the player into the game */
		if (sendcmd() < 0)
			return(-1);

		/* wait for reply */
		polldaemon(1);

		switch((int)msgbuf[msgstart]) {
			case UmINIT:	/* pick a side or a ship */
				(void)getmsgch();
				/* print the message to stdout */
				while ((c = getmsgch()) != '\0')
					(void)putchar(c);
				(void)gets(cmdbuf);
				if (*cmdbuf == '\0')
					strcpy(cmdbuf, "?");
				cmdstart = 0;
				cmdend = strlen(cmdbuf);
				cmdbuf[cmdend++] = CMDSEP;
				continue;
			case UmPDIED:	/* can't get in for some reason */
				(void)getmsgch();
				(void)printf("Sorry, universe too full\n");
				return(-1);
			default:
				active = 1;
				break;	/* we're in */
		}
		break;
	}

	/* send first command */
	setpwent();
	passwd = getpwuid((int)getuid());
	endpwent();
	(void)sprintf(cmdbuf, "se na Captain %s%c", passwd->pw_name, CMDSEP);
	cmdstart = 0;
	cmdend = strlen(cmdbuf);

	/* send initial user commands */
	if ((s = getenv("UNIWAROPTS")) == NULL)
		return(0);	/* no first command */
	cmdend += strlen(s) + 1;
	if (cmdend >= CMDBUFLEN) {
		Uerror = UEOPTSLEN;
		return(-1);
	}
	(void)strcat(cmdbuf, s);
	cmdbuf[cmdend-1] = CMDSEP;
	return(0);
}

