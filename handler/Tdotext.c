#include <curses.h>	/* curses.h automatically includes stdio.h */
#include "config.h"
#include "uniwar.h"

void textprint();

/*
 * Add text directly to win.  If moveflg is TRUE, move to
 * upper left corner first.
 */
void
dotext(win, moveflg)
register WINDOW *win;
register int moveflg;
{
	register int c;
	register char *cp;
	char buf[1024];	/* we don't take chances */

	if (moveflg == TRUE)
		wmove(win, 0, 0);
	cp = buf;
	while ((c = getmsgch()) != '\0') {
		if (moveflg == TRUE)	/* printing status */
			waddch(win, (char)c);
		else
			*cp++ = c;
	}
	if (moveflg != TRUE) {
		*cp = '\0';
		textprint(buf);
	}
}

