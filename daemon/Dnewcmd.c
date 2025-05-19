#include "config.h"
#include "u.h"

char *memcpy();

/*
 * copy newstr into circular buffer pl->plbuf.cbuf[].
 * pl->plbuf.start and pl->plbuf.end are
 * indices to current start and end of buffer.
 * The buffer is of size MAXPMSG, and the newstr is
 * of length msize (including the trailing NULL).
 * Append a CMDSEP to the string.
 */
newcmd(pl, newstr, msize)
PLAYER *pl;
char *newstr;
int msize;
{
	int room;
	short *cstart, *cend;
	char *buf;

	buf = pl->plbuf.cbuf;
	cstart = &pl->plbuf.start;
	cend = &pl->plbuf.end;

	/* first calculate how much room is left */
	if (*cstart > *cend)
		room = *cstart - *cend;
	else
		room = MAXPMSG - (*cend - *cstart);
	if (room < msize)
		return(-1);	/* stack too high; cmd flushed; no warning */

	/* now copy the string and reset cend */
	room = MAXPMSG - *cend;
	if (*cstart > *cend || room >= msize) {
		/* we can do it all with one memcpy */
		(void)memcpy(&buf[*cend], newstr, msize);
		*cend += msize;
		buf[*cend - 1] = CMDSEP;
		if (*cend == MAXPMSG)
			*cend = 0;
	} else {
		/* we need two memcpys */
		(void)memcpy(&buf[*cend], newstr, room);
		*cend = msize - room;
		(void)memcpy(buf, &newstr[room], *cend);
		buf[*cend - 1] = CMDSEP;
	}
	return(0);
}
