#include <curses.h>
#include "config.h"
#include "uniwar.h"

extern WINDOW	*mapwin,	/* window containing map (short-range scan) */
		*cmdwin;	/* window containing user's typed commands */

extern int	Uerror;		/* stores uniwar error type for error() */
extern int	Echar, Kchar;	/* erase and kill chars for this user */
extern short	cmdstart,	/* first byte to process in cmdbuf buffer */
		cmdend;		/* last byte to process in cmdbuf buffer */

extern char cmdbuf[];	/* circular buffer for outgoing commands */
extern char Outbuf[];	/* buffer for outgoing commands */

void error(), doshell();
unsigned sleep();

char *strcpy();

/*
 * getcmdch() - read a char and put it on the command line.  Keep
 * track of where we are in 'cmdx'.  Store character in thiscmd[].
 * When '\r' (RETURN) is read, copy thiscmd[] to prevcmd[] and
 * transfer prevcmd[] to cmdbuf[] by calling circlecpy().  If CMDREPEAT
 * (ESC) is the first char typed, directly copy thiscmd[] to cmdbuf[].
 * Thus, the previous command can be repeated by entering CMDREPEAT.
 *
 * The CMDREDRAW character (when entered at the beginning of a line)
 * redraws the screen.
 *
 * The CMDSHELL character (when entered at the beginning of a line)
 * causes a shell escape.
 *
 * Commands are separated by CMDSEP characters.  RETURN is translated
 * to a CMDSEP character in the buffer.
 */ 
getcmdch()
{
	static int cmdx;
	static int prevcnt;	/* how many times did we do prev cmd? */
	static char thiscmd[79];
	static char prevcmd[79];
	int c;			/* current char read */

	if (cmdx == 0) {
		wmove(cmdwin, 1, 2);	/* 2 to get past "> " */
		wclrtoeol(cmdwin);	/* in case got here via interrupt */
		wrefresh(cmdwin);
	}

	while ((c = getch()) != '\r' && c != CMDREPEAT) {
		if (c == -1) {	/* no input just now */
			(void)sleep(1);	/* maybe 2 would be better! */
			return(0);	/* go poll the daemon */
		}

		else if (c == CMDREDRAW)
			clearok(mapwin, TRUE);
		else if (c == CMDSHELL && cmdx == 0)
			doshell();
		else if (c == Echar) {	/* erase character typed */
			if (cmdx > 0) {
				cmdx--;
				/* cmdx+2 to get past the "> " prompt */
				wmove(cmdwin, 1, cmdx+2);
				wclrtoeol(cmdwin);
			}
		} else if (c == Kchar) {	/* kill character typed */
			wmove(cmdwin, 1, 2);	/* get past "> " prompt */
			wclrtoeol(cmdwin);
			cmdx = 0;
		/*
		 * don't allow control chars other than the above, and
		 * don't allow first character to be a CMDSEP.
		 */
		} else if (c < ' ' || c > '\177' || cmdx >= 76 ||
					(c == CMDSEP && cmdx == 0))
			beep();
		else {
			thiscmd[cmdx++] = c;
			mvwaddch(cmdwin, 1, cmdx+1, (char)c);
		}
		wrefresh(cmdwin);
	}

	/* send a command */
	if (cmdx == 0) {	/* send previous command or do nothing */
		if (c == CMDREPEAT) {
			/* don't allow repeated "tell" commands --
			   if user hasn't typed anything yet, can't repeat */
			if (((prevcmd[0] == 'T' || prevcmd[0] == 't') &&
				(prevcmd[1] == 'E' || prevcmd[1] == 'e')) ||
				prevcmd[0] == '\0') {
				/* special case -- no previous cmd */
				beep();
				return(0);
			}
			prevcnt++;
		} else	/* hit RETURN on empty line -- NULL command */
			return(0);
	} else {		/* send new command */
		(void)strcpy(prevcmd, thiscmd);
		prevcnt = 1;
		prevcmd[cmdx++] = CMDSEP;
		prevcmd[cmdx] = '\0';
		cmdx = 0;
	}
	if (circlecpy(prevcmd, cmdbuf, &cmdstart, &cmdend,
			CMDBUFLEN, strlen(prevcmd)) < 0) {
		/* won't fit; this command won't get executed */
		Uerror = UECMDFULL;
		error("");
		beep();
	}
	mvwprintw(cmdwin, 0, 0, "[%d] %s", prevcnt, prevcmd);
	wclrtobot(cmdwin);
	mvwprintw(cmdwin, 1, 0, "> ");
	wrefresh(cmdwin);
	return(0);
}

