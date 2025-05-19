#include <ctype.h>
#include <curses.h>	/* curses.h automatically includes stdio.h */
#include "config.h"
#include "uniwar.h"

extern WINDOW	*textwin;	/* window containing text messages */

void textchr();

/*
 * print string, bumping linenum for each new line.
 * Keep words from spanning lines.
 */
void
textprint(s)
register char *s;
{
	static int col;
	char *cp;

	while (*s != '\0') {
		/* first, print leading whitespace */
		while(isspace(*s)) {
			if (*s == '\n') {
				textchr((int)(*s));
				col = 0;
			} else if (col < TEXTCOLS) {
				textchr((int)(*s));
				col++;
			}
			s++;
		}

		/* find end of next word */
		for (cp = s; !isspace(*cp) && *cp != '\0'; cp++)
			;
		if (col + (int)(cp-s) >= TEXTCOLS) {
			textchr('\n');
			textchr(' ');
			col = 1;
		}

		/* print the next word */
		while(!isspace(*s) && *s != '\0') {
			textchr((int)(*s));
			col++;
			s++;
		}
	}
	wrefresh(textwin);
}

