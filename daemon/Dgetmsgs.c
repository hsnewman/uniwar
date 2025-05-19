#include <stdio.h>
#ifdef DEBUG
#include <ctype.h>
#endif
#include <fcntl.h>
#include "errno.h"
#include "config.h"
#include "u.h"

extern char Buffer[];
extern short npl, npg;
extern short availship, usedship;
extern PLAYER ships[];
extern int Uerror;
extern short msgempty;
extern int Ifd;		/* input FIFO */
extern int Mrows, Mcols;

extern int errno;

PLAYER *getpl();
void newbaud();
long getplinfo();
unsigned sleep();
long time();
char *memcpy(), *strcat();
void lockscore();
void unlockscore();

/*
 * Empty the FIFO file, adding individual msgs to the correct
 * player's buffer.  If first short of msg is 0, we got a new
 * player for the pregame (next short is player's pid).  Otherwise,
 * first short is player's pid.
 */
getmsgs()
{
#ifdef DEBUG
	short i;
#endif
	short uid;
	short plpid;
	short plx;
	PLAYER *pl;
	PLAYER tpl;
	static int textsize;
	short msize;
	char ofifonam[18];
	char *cp;
	struct scorefile plscore;
	long seekaddr;
	long thistime;
	char timebuf[16];

	/* read the FIFO file */
	for (;;) {
		if ((textsize = read(Ifd, Buffer, MAXMSG)) < 0) {
			/* if error other than "interrupt" (due to alarm) */
			if (errno == EINTR)
				continue;
			Uerror = UEUNIX;
			return(-1);
		}
		if (textsize == 0) {
			if (msgempty == TRUE) {
				/* no msgs to process; sleep and try again */
				(void)sleep(1);
				continue;
			} else
				break;
		}
#ifdef DEBUG
(void)printf("read %d bytes from input FIFO\n\ttext: ", textsize);
cp = Buffer;
for (i = 0; i < textsize; i++) {
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
(void)fflush(stdout);
#endif
		/* msize is the size of the command text -- set in loop */
		for (cp = Buffer; cp < Buffer + textsize; cp += msize) {
			(void)memcpy((char *)(&plpid), cp, sizeof(short));
			cp += sizeof(short);
			if (plpid != 0) {
				/* set msize (sizeof msg) */
				(void)memcpy((char *)(&msize), cp,
							sizeof(short));
				cp += sizeof(short);

				/*
				 * newcmd:
				 * if player's command buffer is full, send
				 * error.  otherwise, add the new command to
				 * player's buffer.
				 */
				if ((pl = getpl(plpid)) == NULL) {
					/* probably stale msg from dead
					   player -- ignore it */
					continue;
				}

				/* is it a special "do now" message? */
				/* specmsg() doesn't care about size */
				switch(specmsg(pl, cp)) {
				case -1:
					return(-1);	/* error */

				case 0:
					/* not a special message */
					if (newcmd(pl, cp, (int)msize) < 0)
						/* player's stack too high;
						 * this command is lost! */
						continue;

					/* got msg, don't wait-loop for more */
					msgempty = FALSE;
					break;

				case 1:	/* special msg */
					break;

				case 2:	/* special msg, but msgempty should be
					   FALSE (e.g., pl stopped waiting) */
					msgempty = FALSE;
					break;

				default:
					break;
				}
			} else {
				/* new pregamer */
				msize = sizeof(short);
				(void)memcpy((char *)(&plpid), cp,
							sizeof(short));
				cp += sizeof(short);
				(void)memcpy((char *)(&uid), cp,
							sizeof(short));
				(void)sprintf(ofifonam, "/tmp/UnIwAr%05d",
							(int)plpid);
				if ((tpl.ofd =
					open(ofifonam
						, O_WRONLY | O_NDELAY)) < 0) {
					Uerror = UEUNIX;
					return(-1);
				}
				tpl.pending = 0;
				if (npl >= MAXPL) {
					if (pmsg(UmPDIED, &tpl, "", 0) < 0) {
						(void)close(tpl.ofd);
						return(-1);
					}
					(void)close(tpl.ofd);
					continue;
				}
				/* see where pl uid lives in score file */
				lockscore();	/* lock scorefile */
				seekaddr = getplinfo(&plscore, uid);
				thistime = time(0L);
				/* tell pl how much longer to wait */
				if (thistime - plscore.timelast
					< MINWAITIME)
					(void)sprintf(timebuf, "%d"
						, MINWAITIME -
						(thistime - plscore.timelast));
				else
					(void)sprintf(timebuf, "0");

				if (plscore.timelast == 0) {
					/* brand new player */
					plscore.uid = uid;
					(void)strcat(plscore.name, "New");
					plscore.highest = 0;
					plscore.total = 0;
					plscore.timelast = thistime;
					plscore.mission = 0;
					/* create new entry */
					(void)putplinfo(&plscore, seekaddr);
				}
				unlockscore();	/* unlock scorefile */
				if (pmsg(UmSLEEP, &tpl, timebuf, -1) < 0) {
					(void)close(tpl.ofd);
					return(-1);
				}

				/* tell the player galaxy dimensions */
				sprintf(timebuf, "%d %d", Mrows, Mcols);
				if (pmsg(UmROWCOL, &tpl, timebuf, -1) < 0) {
					(void)close(tpl.ofd);
					return(-1);
				}

				/* init next spot in player structure */
				plx = availship;
				pl = &ships[plx];
				/* update free list */
				availship = pl->list;
				pl->list = usedship;

				/* update used list */
				usedship = plx;

				pl->ofd = tpl.ofd;
				pl->pid = plpid;
				pl->uid = uid;
				pl->flags = 0;
				pl->plbuf.start = 0;
				pl->plbuf.end = 0;
				npl++;	/* one more player in game */
				npg++;	/* one more player in pregame */
			}
		}
	}
	return(1);
}

/*
 * wait until every player has said GOODBYE.
 */
waitforgoodbyes()
{
	register PLAYER *pl;
	int textsize;
	short msize;
	short plpid;
	char *cp;

	while(npl) {
		sleep(2);	/* don't spin while players are
				   reading their screens */

#ifdef DEBUG
(void)printf("waitforgoodbyes: npl=%d\n", npl);
#endif
		/* wait for messages from players */
		if ((textsize = read(Ifd, Buffer, MAXMSG)) < 0) {
			/* if error other than "interrupt" (due to alarm) */
			if (errno == EINTR)
				continue;
			Uerror = UEUNIX;
			return(-1);
		}

		/* msize is the size of the command text -- set in loop */
		for (cp = Buffer; cp < Buffer + textsize; cp += msize) {
			(void)memcpy((char *)(&plpid), cp, sizeof(short));
			cp += sizeof(short);
			if (plpid == 0) {
				/* ignore the new player */
				msize = sizeof(short);
				continue;
			}
			/* set msize (sizeof msg) */
			(void)memcpy((char *)(&msize), cp, sizeof(short));
			cp += sizeof(short);
			if (*cp == GOODBYE) {
				if ((pl = getpl(plpid)) == NULL) {
					Uerror = UEINTL;
					return(-1);
				}
				/* clean up player and decrement npl */
				pl->flags |= DIED;
				(void)saybyebye(pl);
				if (killpl(pl) < 0)
					return(-1);
#ifdef DEBUG
(void)printf("waitforgoodbyes: killed a player\n");
#endif
			}
			/* ignore the message otherwise */
		}
	}
	return(1);
}

/*
 * getpl returns pointer to the PLAYER structure with pid == p
 */
PLAYER *
getpl(p)
register short p;
{
	register PLAYER *pl;
	register int i;

	for (i = usedship; i != -1; i = pl->list) {
		pl = &ships[i];
		if (pl->pid == p)
			return(pl);
	}
	return((PLAYER *)NULL);
}

/*
 * specmsg -- check for a special "do now" message.
 * If we find one, return 1, else return 0.  On error,
 * return -1.
 */
specmsg(pl, cp)
register PLAYER *pl;
register char *cp;
{
	register int ret;
	int baud;

	ret = 1;
	switch((int)cp[0]) {
	case ABORT:
		/* abort stacked commands */
		pl->plbuf.start = pl->plbuf.end = 0;
		break;

	case GOODBYE:
		/* player is GONE, has already been said byebye to
		   unless player was never active in which case we
		   DON'T say byebye! */
		/* (void)saybyebye(pl);	*/
		if (killpl(pl) < 0)
			return(-1);
		break;

	case RESPCHR:
		/* responding to msg from daemon */
		pl->pending--;
		pl->flags &= ~WAITING;
		if (pl->plbuf.start != pl->plbuf.end)
			return(2);	/* sets msgempty to FALSE */
		break;

	case BAUDCHR:
		(void)memcpy((char *)(&baud), cp+1, sizeof(int));
		if (pl->baudrate != baud)
			newbaud(pl, baud);
		break;

	default:
		ret = 0;
		break;
	}
	return(ret);
}
