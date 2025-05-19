#include <curses.h>	/* curses.h automatically includes stdio.h */
#include "config.h"
#include "uniwar.h"

#ifdef DEBUG
extern WINDOW	*statwin;	/* window containing ship's status */
#endif
extern WINDOW	*textwin;	/* window containing text messages */
extern WINDOW	*cmdwin;	/* window containing commands typed */

extern short	linenum;	/* current line in text window */
extern short	doprompt;	/* prompt for --more-- in text window */

/*
 * put character in text window.  New line character gets
 * special treatment.
 */
void
textchr(c)
register int c;
{
	if (c == '\n') {
		/* in the text window */
		linenum++;
		wmove(textwin, linenum, 0);
		if (linenum == 5)	/* arbitrary */
			wclrtobot(textwin);
		else
			wclrtoeol(textwin);
		if (linenum == TEXTROWS-1) {
			if (doprompt) {
				wattron(textwin, A_REVERSE|A_STANDOUT);
				waddstr(textwin, "--more--");
				wattroff(textwin, A_REVERSE|A_STANDOUT);
				wrefresh(textwin);
				nodelay(cmdwin, FALSE);
				(void)getch();
				nodelay(cmdwin, TRUE);
				wmove(textwin, linenum, 0);
				wclrtoeol(textwin);
			}
			linenum = 0;
			wmove(textwin, linenum, 0);
			wclrtoeol(textwin);
		}
/*
#ifdef DEBUG
mvwprintw(statwin, 0, 0, "linenum=%d", linenum);
#endif
*/
	} else
		waddch(textwin, (char)c);
}

