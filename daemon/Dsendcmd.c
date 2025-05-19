#include <errno.h>
#ifdef DEBUG
#include <ctype.h>
#endif
#include "config.h"
#include "u.h"

extern int errno;

extern int Uerror;
extern char Outbuf[];

unsigned sleep();
char *memcpy();

/*
 * sendcmd -- send one command from daemon to player pl.
 * Size is msize.  Message should already be in Outbuf+sizeof(short).
 * Encode the size in the first short.
 */
sendcmd(pl, msize)
register PLAYER *pl;
register int msize;
{
	int ret;
	short tmp;
#ifdef DEBUG
int i;
char *cp;
#endif
	if (pl->pending >= MAXPEND && Outbuf[sizeof(short)] != UmPDIED) {
		/* check to see if player process is dead */
pldead:
		if (kill(pl->pid, 0) == -1) {
			if (errno == ESRCH) {	/* he's dead, Jim */
				/* don't update score file for
				   players who terminate abnormally */
				pl->flags |= DIED;	/* may need it */
				(void)saybyebye(pl);	/* remove from map */
				return(killpl(pl));
			}
		}
		return(1);	/* sorry, too full */
	}
#ifdef DEBUG
(void)printf("msg of size %d to pid %d\n\ttext: ", msize, pl->pid);
cp = Outbuf+sizeof(short);
for (i = 0; i < msize; i++) {
	if ((isascii((int)(*cp)) && *cp < '\177' && *cp >= ' ') ||
					isspace((int)(*cp)))
		(void)putchar(*cp);
	else
		(void)printf("\\%03o", *cp);
	if (*cp == '\n')
		(void)printf("\t\t");
	cp++;
}
(void)putchar('\n');
#endif
	tmp = (short)msize;
	(void)memcpy(Outbuf, (char *)(&tmp), sizeof(short));
	for (;;) {
		if ((ret = write(pl->ofd, Outbuf
				, (unsigned)(msize+sizeof(short)))) < 0) {
			if (errno == EPIPE)	/* player proc died */
				goto pldead;
			Uerror = UEUNIX;
			return(-1);
		} else if (ret != msize+sizeof(short))
			/* pipe full, wait a sec */
			(void)sleep(1);
		else
			break;
	}
	pl->pending++;
	return(1);
}
