#include "config.h"
#include "u.h"

extern struct stats stats;
extern char Outbuf[];
extern int Fastgame;

char *memcpy();

/*
 * Send a message with "type" encoded in the front to pl.
 * if size is -1, use strlen() to compute size.
 * All messages leave first short clear for sendcmd() to
 * save the size.
 */
pmsg(type, pl, str, size)
int type;
register PLAYER *pl;
register char *str;
register int size;
{
	if (size == -1)
		size = strlen(str)+1;	/* 1 for trailing NULL */
	Outbuf[sizeof(short)] = (char)type;
	(void)memcpy(Outbuf+1+sizeof(short), str, size);
	return(sendcmd(pl, size+1));	/* add 1 for leading type */
}

/*
 * psleep -- Put a player to sleep for sec seconds.  Return 2 if no sleep.
 */
psleep(pl, sec)
PLAYER *pl;
int sec;
{
	if (stats.Ecnt + stats.Fcnt == 1 || Fastgame)
		/* no sleep if you're alone or fastgame enabled */
		return(2);
	Outbuf[sizeof(short)] = UmSLEEP;
	Outbuf[sizeof(short)+1] = (char)sec;
	pl->flags |= WAITING;
	return(sendcmd(pl, 2));
}
