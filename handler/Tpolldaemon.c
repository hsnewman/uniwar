#include <curses.h>
#include <errno.h>
#include <signal.h>
#include "config.h"
#include "uniwar.h"

extern WINDOW *cmdwin;		/* window containing user's typed commands */

extern short	died;		/* TRUE if player or daemon died */
extern int	Uerror;		/* stores uniwar error type for error() */
extern short	mypid;		/* term handler's process id */
extern short	msgstart,	/* first byte to process in msgbuf buffer */
		msgend;		/* last byte to process in msgbuf buffer */
extern short	active;		/* player is activated in daemon's eyes */
extern int	Mrows,		/* number of rows in galaxy */
		Mcols;		/* number of columns in galaxy */

extern char msgbuf[];	/* circular buffer for incoming messages */
extern char Outbuf[];
extern char Buffer[];
extern int Ifd;

extern int errno;
extern int interrupted;
unsigned alarm(), sleep();
char *memcpy();
void sweetdreams();

void	enditall(), error(), msghandle(), textprint(), getmsgsht();

/*
 * polldaemon -- argument n can be positive, 0, or negative.
 * If n is positive, wait forever for first n messages pending for
 * this process and put them in the message holding area msgbuf[].
 * Don't process them.
 *
 * If n is negative, get all messages currently pending and process
 * them.
 *
 * If n is 0, wait forever for first message, then get any other
 * messages pending.  Process all messages received.
 */
void
polldaemon(n)
register int n;
{
	int (*oldsig)();
	register int size;	/* bytes received by read */
	register int awaitmsg;
	int count;		/* don't read more than 3 msgs */
	short msize;		/* length of individual msg */
	short stopflg;
	int sleeptime;
	char *cp;

	stopflg = 0;
	if (n > 0)
		stopflg = 1;	/* stop reading messages when n reaches 0 */
	else if (n == 0)
		n++;

	/* don't read more than three msgs -- give user a chance to type! */
	for (count = 0; count < 3; count++) {
		if (n > 0) {
			awaitmsg = 1;	/* wait "forever" */
			n--;
		} else if (n == 0 && stopflg == 1)
			break;
		else {
			awaitmsg = 0;	/* just poll, don't wait */
			n--;
		}

		/* wait for message from daemon */
		interrupted = 0;
		while (!interrupted) {
			/* prevent screwing up read */
			oldsig = signal(SIGINT, (int(*)())(SIG_IGN));
			if ((size = read(Ifd, Buffer, MSGBUFLEN)) < 0) {
				Uerror = UEUNIX;
				enditall(0);
			}
			(void)signal(SIGINT, oldsig);
			if (size == 0 && awaitmsg)
				sleep(1);
			else
				break;
		}
		interrupted = 0;

		/* if we got out of the for loop with size 0, no msgs */
		if (size == 0)
			break;

		/*
		 * buffer up each message.  Format of message:
		 *
		 * short msgsize followed by message text.
		 *
		 * msize is set in loop.
		 */
		for (cp = Buffer; cp < Buffer+size; cp += msize) {
			memcpy((char *)(&msize), cp, sizeof(short));
			cp += sizeof(short);

			/* don't buffer up messages that say we died. */
			if (*cp == UmPDIED) {
				msghandle();	/* see if we get pts now */
				died = 1;
				continue;
			}

			if (*cp == UmROWCOL)
				sscanf(cp+1, "%d %d", &Mrows, &Mcols);

			/* this code is only used if not using
			   SIGARLM in daemon */
			if (*cp == UmSLEEP) {
				if (!active) {
					/* gotta wait ! */
					if (cp[1] != '0') {
						sweetdreams(atoi(cp+1));
					}
					continue;
				}
				msghandle();	/* output before sleeping */
				sleeptime = (int)cp[1];
				wattron(cmdwin, A_REVERSE|A_STANDOUT);
				mvwprintw(cmdwin, 1, 2, "WAIT %d", sleeptime);
				wrefresh(cmdwin);
				wattroff(cmdwin, A_REVERSE|A_STANDOUT);
				while (sleeptime =
					(int)sleep((unsigned)sleeptime))
					; /* in case user hits interrupt */
				wmove(cmdwin, 1, 2);
				wclrtoeol(cmdwin);
				wrefresh(cmdwin);
			}

			/* send response (no response to UmPDIED) */
			Outbuf[2*sizeof(short)] = RESPCHR;
			if (write_it((short)1) < 0) {
				Uerror = UEUNIX;
				enditall(0);
			}

			/*
			 * don't buffer up sleeps or when galaxy
			 * dimensions are reported
			 */
			if (*cp == UmSLEEP || *cp == UmROWCOL)
				continue;

			if (circlecpy(cp, msgbuf, &msgstart, &msgend
				, MSGBUFLEN, (int)msize) < 0) {
				/* won't fit; this message will be ignored */
				Uerror = UEMSGFULL;
				error("");
				beep();
				break;	/* don't read more msgs just now */
			}
		}
	}
	if (n < 0) {
		/*
		 * Handle all messages from daemon.  Hitting SIGINT causes
		 * all incoming messages to be flushed.
		 */
		msghandle();	/* process buffered messages */
	}
	if (died)
		enditall(0);
}

/*
 * sweetdreams -- sleep stime seconds, but entertain the user periodically.
 *		  windows aren't up yet, so we can backspace.
 */
void
sweetdreams(stime)
int stime;
{
	printf("You must wait before you can reenter the game.\n");
	printf("Time remaining: ");
	while(stime > 0) {
		printf("%3d", stime);
		fflush(stdout);
		if (stime > 5)
			sleep(5);
		else
			sleep(stime);
		stime -= 5;
		printf("\b\b\b");
	}
	printf("\n\n\n");
}
