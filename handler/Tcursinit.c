#include <stdio.h>
#include <curses.h>	/* curses.h automatically includes stdio.h */
#include "config.h"
#include "uniwar.h"

extern WINDOW	*mapwin,	/* window containing map (short-range scan) */
		*statwin,	/* window containing ship's status */
		*textwin,	/* window containing text messages */
		*cmdwin;	/* window containing user's typed commands */

extern int	Echar, Kchar;	/* erase and kill chars for this user */
extern int	Uerror;
extern short	wins_on;	/* flag TRUE when initscr() has been called */
extern char Outbuf[];

char *memcpy();
void enditall();

/*
 * Initialize windows.
 */
cursinit()
{
	int brate;	/* baudrate */

	initscr();	/* required initialization for curses */
	if (LINES < 24 || COLS < 80) {
		endwin();
		printf("sorry, screen must be at least 24x80\n");
		return(-1);
	}
	wins_on = TRUE;	/* it's safe to call endwin() now */
	cbreak();	/* enter CBREAK mode */
	noecho();	/* don't echo typed chars as typed */
	nonl();		/* speed up curses use of <cr> and <lf> */

	/* don't refresh these until they've been initialized */
	mapwin = subwin(stdscr, MAPROWS, MAPCOLS, MAPBEGY, MAPBEGX);
	statwin = subwin(stdscr, STATROWS, STATCOLS, STATBEGY, STATBEGX);
	textwin = subwin(stdscr, TEXTROWS, TEXTCOLS, TEXTBEGY, TEXTBEGX);
	cmdwin = subwin(stdscr, CMDROWS, CMDCOLS, CMDBEGY, CMDBEGX);
	/* Screen will be refreshed by the play() routine */

	leaveok(mapwin, TRUE);
	leaveok(statwin, TRUE);
	leaveok(textwin, TRUE);

	nodelay(cmdwin, TRUE);	/* make getch() non-blocking */

	/*** not used
	idlok(textwin, TRUE);
	scrollok(textwin, TRUE);
	***/

	Echar = erasechar();
	Kchar = killchar();

	/* tell the daemon what our baudrate is */
	Outbuf[2*sizeof(short)] = BAUDCHR;	/* probably ^E */
	brate = baudrate();
	(void)memcpy(Outbuf + 2*sizeof(short) + 1, (char *)(&brate)
						, sizeof(int));

	if (write_it((short)(sizeof(int) + 1)) < 0) {
		Uerror = UEUNIX;
		enditall(0);
	}

	return(0);
}
