#include "config.h"
#include "u.h"

/*
 * coordstr - return a pointer to a string describing coordinates
 * of object at (r, c) relative to player pl.
 */
char *
coordstr(pl, r, c)
register PLAYER *pl;
register short r, c;
{
	static char buf[64];

	*buf = '\0';
	if (pl->flags & ABSOOUT)	/* need absolute coords */
		(void)sprintf(buf, "%s%d-%d", (pl->flags & OSHORT) ? "" : "@"
								, r, c);
	if (pl->flags & RELOUT)		/* need relative coords */
		(void)sprintf(&buf[strlen(buf)], " %+d,%+d"
					, r - pl->rpos, c - pl->cpos);
	return(buf);
}
