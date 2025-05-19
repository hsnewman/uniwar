#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <signal.h>
#include "config.h"
#include "uniwar.h"

extern int	Uerror;		/* stores uniwar error type for error() */
extern short	cmdstart,	/* first byte to process in cmdbuf buffer */
		cmdend;		/* last byte to process in cmdbuf buffer */
extern short	mypid;
extern int	Ofd;
extern int	interrupted;

extern char cmdbuf[];	/* circular buffer for outgoing commands */
extern char Outbuf[];	/* buffer for outgoing commands */

char *memcpy();

/*
 * sendcmd -- send all commands in cmdbuf[] to daemon.  Send one command
 * per message.  If there is nothing in cmdbuf[], this is the first msg.
 */
sendcmd()
{
	register char *to, *from;
	register short msize;

	if (cmdstart == cmdend)
		return(write_it((short)0));

	/* some commands to send */
	interrupted = 0;
	while (cmdstart != cmdend) {
		from = &cmdbuf[cmdstart];
		for (to = Outbuf+2*sizeof(short), msize = 0; *from != CMDSEP
					; to++, msize++) {
			if (interrupted)
				/* need sure way out of loop if interrupted */
				break;
			*to = *from;
			cmdstart++;
			if (cmdstart == CMDBUFLEN) {
				cmdstart = 0;
				from = cmdbuf;
			} else
				from++;
		}
		*to = '\0';
		msize++;	/* count the trailing NULL */
		cmdstart++;
		if (cmdstart == CMDBUFLEN)
			cmdstart = 0;

		if (interrupted) {
			/* cmdstart/cmdend may be messed up */
			cmdstart = cmdend = 0;
			interrupted = 0;
		} else if (write_it(msize) < 0) {
			Uerror = UEUNIX;
			return(-1);
		}
	}
	return(0);
}

/*
 * write_it -- there is a message stored at Outbuf + 2*sizeof(short).
 * The msg is of length msize.  Encode the player's pid with the
 * message size in the first two shorts of Outbuf, and write the
 * Outbuf to the daemon's input FIFO.
 *
 * If msize is 0, this is a special case (new player) -- in this
 * case, store 0 in first short, player's pid in second short, and
 * player's uid in third short.
 */
write_it(msize)
short msize;
{
	int (*oldsig)();
	short tmp = 0;
	short uid;

	if (msize == 0) {
		/* special case */
		memcpy(Outbuf, (char *)(&tmp), sizeof(short));
		memcpy(Outbuf+sizeof(short), (char *)(&mypid), sizeof(short));
		uid = getuid();
		memcpy(Outbuf+2*sizeof(short), (char *)(&uid), sizeof(short));
		msize = 2;	/* to pick up uid */
	} else {
		memcpy(Outbuf, (char *)(&mypid), sizeof(short));
		memcpy(Outbuf+sizeof(short), (char *)(&msize), sizeof(short));
	}
	/* prevent screwing up write */
	oldsig = signal(SIGINT, (int(*)())(SIG_IGN));
	if (write(Ofd, Outbuf, (unsigned)(msize + 2*sizeof(short))) < 0) {
		Uerror = UEUNIX;
		return(-1);
	}
	(void)signal(SIGINT, oldsig);
	return(0);
}
