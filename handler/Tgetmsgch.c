#include "config.h"
#include "uniwar.h"

extern int	Uerror;		/* stores uniwar error type for error() */
extern short	msgstart,	/* first byte to process in msgbuf buffer */
		msgend;		/* last byte to process in msgbuf buffer */

extern char msgbuf[];	/* circular buffer for incoming messages */

void enditall();

/*
 * getmsgch -- get one character from msgbuf[], updating
 * msgstart accordingly.  Exit if end of buffer (internal error).
 */
getmsgch()
{
	register int c;

	if (msgstart == msgend) {
		Uerror = UEINTL;
		enditall(0);
	}
	c = (int)msgbuf[msgstart];
	msgstart++;
	if (msgstart == MSGBUFLEN)
		msgstart = 0;
	return(c);
}

/*
 * getmsgsht -- assume that there is a short in msgbuf[] instead
 * of a character
 */
void
getmsgsht(c)
register short *c;
{
	register char *cp;
	register int i;

	cp = (char *)c;
	for (i = 0; i < sizeof(short); i++)
		*cp++ = (char)getmsgch();
}

/*
 * getmsglng -- assume that there is a long in msgbuf[] instead
 * of a character
 */
void
getmsglng(c)
register long *c;
{
	register char *cp;
	register int i;

	cp = (char *)c;
	for (i = 0; i < sizeof(long); i++)
		*cp++ = (char)getmsgch();
}

