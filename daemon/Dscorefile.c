#include <stdio.h>
#include <errno.h>
#include <fcntl.h>
#include "config.h"
#include "u.h"

extern int errno;
extern int Dpid;
extern char Lockfile[];
extern char Scorefile[];
long time();
long lseek();
unsigned sleep();
char *memset(), *strcpy();

static int lockfd = -1;

/*
 * lockscore -- lock score file using a simple file semaphore.
 *		If this fails, something weird is going on, so
 *		locking disabled.
 */
void
lockscore()
{
	int tries;
	char pidbuf[32];

	for (tries = 0; tries < 15; tries++) {
		if ((lockfd = open(Lockfile, O_RDWR|O_CREAT|O_EXCL,0700)) < 0) {
			if (errno != EEXIST)
				return;	/* weird - locking disabled */
			(void)sleep(1);
		} else
			break;
	}
	if (tries >= 15)
		/* hmm, lockfile exists but we can't seem to get it.
		   guess we'll pretend we did. */
		return;

	/* write our pid to the file */
	sprintf(pidbuf, "%d", Dpid);
	if (write(lockfd, pidbuf, strlen(pidbuf)) < 0) {
		/* silly program! */
		(void)close(lockfd);
		(void)unlink(Lockfile);
		return;
	}

	/* leave file open on purpose -- used by unlockscore() */
	return;
}

/*
 * unlockscore -- unlock score file using a simple file semaphore.
 *		  If our pid isn't in the file, locking disabled.
 */
void
unlockscore()
{
	char buf[32];
	int pid;

#ifdef DEBUG
(void)printf("unlockscore: lockfd=%d\n", lockfd);
#endif
	if (lockfd == -1)
		return;	/* we never locked it */
	
	(void)memset(buf, '\0', 32);
	(void)lseek(lockfd, 0L, 0);
	if (read(lockfd, buf, 31) < 0) {
		/* that's odd, can't read the file we opened */
		lockfd = -1;
		return;
	}
	(void)close(lockfd);
	lockfd = -1;

	pid = atoi(buf);
#ifdef DEBUG
(void)printf("unlockscore: pid=%d, Dpid=%d\n", pid, Dpid);
#endif
	if (pid != Dpid)
		/* Very weird - somehow we opened other daemon's lock file!
		   pretend we're done */
		return;

	/* ok, the the file is there, and it's our lockfile.  zap it */
	(void)unlink(Lockfile);
	lockfd = -1;
	return;
}

/*
 * getplinfo -- get player history to scorefile structure.
 *		scorefile should be locked by calling process.
 */
long
getplinfo(plscore, uid)
struct scorefile *plscore;
short uid;
{
	struct scorefile tmpscore;
	long saveseek, curseek;
	int scorefd;
	int foundslot;	/* flag */
	int readcnt;

	if ((scorefd = open(Scorefile, O_RDWR|O_CREAT, 0666)) < 0) {
		/* can't deal with scorefile, so clear plscore */
		(void)memset((char *)plscore, '\0', sizeof(struct scorefile));
		return(-1);
	}
	saveseek = -1;
	curseek = 0;
	foundslot = 0;
	tmpscore.uid = -1;
	while ((readcnt = read(scorefd, (char *)(&tmpscore)
					, sizeof(struct scorefile)))
					== sizeof(struct scorefile)) {
#ifdef DEBUG
(void)printf("scorefile entry:\n");
			printf("\t%-16s%8d%16d%16d%8d\n"
				, tmpscore.name
				, tmpscore.mission
				, tmpscore.total/100
				, tmpscore.highest/100
				, tmpscore.uid);
fflush(stdout);
#endif
		if (tmpscore.uid == uid) {
#ifdef DEBUG
(void)printf("uid match, found it!\n");
#endif
			saveseek = curseek;
			break;
		}
		if (tmpscore.uid == -1 && foundslot == 0)
			saveseek = curseek;

		curseek += sizeof(struct scorefile);
	}
	/* either read error, end of file, or found the player. */
	if (readcnt == 0 && saveseek == -1)	/* EOF and no empty spots*/
		saveseek = curseek;

	(void)close(scorefd);

	/* new player? */
	if (tmpscore.uid != uid) {
		plscore->timelast = 0;
		return(saveseek);	/* return first unused slot */
	}
	memcpy((char *)plscore, (char *)(&tmpscore), sizeof(struct scorefile));
	return(saveseek);

}

/*
 * putplinfo -- put player history from scorefile structure.
 *		scorefile should be locked by calling process.
 */
putplinfo(plscore, seekaddr)
struct scorefile *plscore;
long seekaddr;
{
	int scorefd;

	if (seekaddr < 0)
		return(0);	/* No-op */
	if (seekaddr % sizeof(struct scorefile) != 0)
		return(-1);	/* funky seekaddr */
	if ((scorefd = open(Scorefile, O_RDWR)) < 0)
		return(-1);	/* can't open scorefile */
	if (lseek(scorefd, seekaddr, 0) < 0)
		return(-1);	/* can't seek! */
	if (write(scorefd, (char *)plscore, sizeof(struct scorefile))
			!= sizeof(struct scorefile))
		return(-1);	/* write error */

	(void)close(scorefd);
	return(0);
}

/*
 * updatescore -- put player history from scorefile structure.
 *		  scorefile should be locked by calling process.
 *
 *		  lock/unlock scorefile from calling process
 *		  since there are times (e.g., game ending
 *		  in dopoint()) when we call this many times
 *		  in a row and don't want locking overhead.
 */
updatescore(pl)
PLAYER *pl;
{
	struct scorefile plscore;
	long seekaddr;

	seekaddr = getplinfo(&plscore, pl->uid);
#ifdef DEBUG
(void)printf("updatescore: scorefile seekaddr=%d\n", seekaddr);
#endif
	if (plscore.uid != pl->uid) {
#ifdef DEBUG
(void)printf("updatescore: scorefile uid=%d, player uid=%d\n", plscore.uid, pl->uid);
(void)printf("scorefile entry:\n");
			printf("\t%-16s%8d%16d%16d%8d\n"
				, plscore.name
				, plscore.mission
				, plscore.total/100
				, plscore.highest/100
				, plscore.uid);
fflush(stdout);
#endif
		/* that's odd, an entry for this guy should
		   have been created by getmsgs().  There must
		   be a good reason why this player's entry
		   is missing, so let's not do anything. */
		return(-1);
	}

	/* update the plscore struct with info from this game */
	(void)strcpy(plscore.name, pl->name);
	if (plscore.highest < pl->score[STOTAL])
		plscore.highest = pl->score[STOTAL];
	plscore.total += pl->score[STOTAL];
	plscore.timelast = time(0L);
	plscore.mission++;

	(void)putplinfo(&plscore, seekaddr);
	return(0);
}
