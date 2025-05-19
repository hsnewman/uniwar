#include "config.h"
#include "uniwar.h"

/*
 * coordstr - return a pointer to a string describing coordinates
 * of object at (r, c) relative to player pl.
 */
char *
coordstr(plflags, r, c, rpos, cpos)
long plflags;
register short r, c;
short rpos, cpos;
{
	static char buf[64];

	*buf = '\0';
	if (plflags & ABSOOUT)	/* need absolute coords */
		(void)sprintf(buf, "%s%d-%d", (plflags & OSHORT) ? "" : "@"
								, r, c);
	if (plflags & RELOUT)		/* need relative coords */
		(void)sprintf(&buf[strlen(buf)], " %+d,%+d"
					, r - rpos, c - cpos);
	return(buf);
}
