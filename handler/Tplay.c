extern short	cmdstart,	/* first byte to process in cmdbuf buffer */
		cmdend;		/* last byte to process in cmdbuf buffer */

void polldaemon();

/*
 * Here we have a simple loop:
 *
 * for(;;) {
 *	If user typed anything, read what they typed.
 *	If there is a full command to send, send it and remove it
 *		from cmdbuf[].
 *	Collect any incoming messages from daemon to msgbuf[], and
 *		process them.
 * }
 */
void
play()
{
	/* read all messages already sent from daemon */
	polldaemon(-1);

	for (;;) {
		/*
		 * getcmdch() returns when player hits RETURN.
		 */
		if (getcmdch() < 0)
			return;	/* error reading command from player */
		
		if (cmdstart != cmdend) {	/* something to send */
			if (sendcmd() < 0)	/* send all cmds in buffer */
				return;		/* error */
			polldaemon(0);		/* expect some response */
		} else
			polldaemon(-1);		/* check for messages, but
						   don't wait forever */
	}
}

