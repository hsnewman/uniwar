#include <curses.h>	/* curses.h automatically includes stdio.h */
#include <signal.h>
#include "config.h"
#include "uniwar.h"

extern WINDOW	*mapwin,	/* window containing map (short-range scan) */
		*statwin,	/* window containing ship's status */
		*textwin;	/* window containing text messages */

extern int	Uerror;		/* stores uniwar error type for error() */
extern short	msgstart,	/* first byte to process in msgbuf buffer */
		msgend;		/* last byte to process in msgbuf buffer */

extern char tmpbuf[];	/* used for sprintf prior to textwindow prints */

void	enditall(), error(), polldaemon(), do_scan(), dotext(), docode();
void	dopoint();

/*
 * Process all messages in msgbuf[] cache.  All messages begin
 * with a code identifying what the following bytes mean.
 * All of the routines called within the switch reset msgstart,
 * but do not refresh the screen (that is done here).
 *
 * NOTE: startup() routine has its own switch to read msgs since
 * curses have not been initialized yet.
 */
void
msghandle()
{
	int (*oldsig)();
	register int mtype;

	while (msgstart != msgend) {

		/* prevent screwing up buffer */
		oldsig = signal(SIGINT, (int(*)())(SIG_IGN));

		switch (mtype = getmsgch()) {
		case UmSCAN:
			do_scan();
			break;

		case UmSTATUS:
			dotext(statwin, TRUE);	/* TRUE means curs to top */
			break;

		case UmTEXT:
			dotext(textwin, FALSE);
			break;

		case UmCODE:
			docode(FALSE);
			break;

		case UmLIST:
			docode(TRUE);	/* TRUE: use paging prompt */
			break;

		case UmPOINT:
			dopoint();	/* display points */
			break;

		/* can't get UmPDIED -- handled in polldaemon() */

		default:
			Uerror = UEBADMSG;
			(void)sprintf(tmpbuf, "bad msg type %d", mtype);
			error(tmpbuf);
			break;
		}

		(void)signal(SIGINT, oldsig);
	}

	wrefresh(mapwin);
	wrefresh(statwin);
	wrefresh(textwin);
}

