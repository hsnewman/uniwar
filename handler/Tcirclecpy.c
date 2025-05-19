#include <signal.h>

char *memcpy();

/*
 * circlecpy -- copy newstr into circular buffer cbuf.
 * cstart and cend are pointers to current start and end of buffer.
 * csize is size of buffer, strsize is size of newstr.
 *
 * Disable interrupts.
 */
circlecpy(newstr, cbuf, cstart, cend, csize, strsize)
char *newstr, *cbuf;
register short *cstart, *cend;
register int strsize;
int csize;
{
	int (*oldsig)();
	int room;

	/* if we interrupt now, it will screw up the buffers */
	oldsig = signal(SIGINT, (int(*)())(SIG_IGN));

	/* first calculate how much room is left */
	if (*cstart > *cend)
		room = *cstart - *cend;
	else
		room = csize - (*cend - *cstart);
	if (room < strsize) {
		(void)signal(SIGINT, oldsig);
		return(-1);
	}

	/* now copy the string and reset cend */
	room = csize - *cend;
	if (*cstart > *cend || room >= strsize) {
		/* we can do it all with one memcpy */
		(void)memcpy(&cbuf[*cend], newstr, strsize);
		*cend += strsize;
		if (*cend == csize)
			*cend = 0;
	} else {
		/* we need two memcpys */
		(void)memcpy(&cbuf[*cend], newstr, room);
		*cend = strsize - room;
		(void)memcpy(cbuf, &newstr[room], *cend);
	}

	/* enable interrupt */
	(void)signal(SIGINT, oldsig);
	return(0);
}
