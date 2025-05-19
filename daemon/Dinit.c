#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <signal.h>
#include "config.h"
#include "u.h"
#include "errno.h"

extern int Uerror;
extern short msgempty;
extern int nocleanup;	/* set if cleanup should not be done on interrupt */
extern int optind;
extern char *optarg;
extern char Ififonam[];
extern int Gamenum;	/* game number */
extern int Fastgame;	/* set if delays are disabled */
extern int Ifd;		/* input FIFO */
//extern int errno;
extern char Lockfile[];
extern char Scorefile[];
extern int Dpid;	/* pid of daemon process */
extern int Mrows, Mcols;

void initrnd(), enditall();

/*
 * Initialize random number generator.
 * Attach to standard uniwar fifo for this Gamenum.
 * Ignore all signals.
 *
 * options: -a enable alarm
 *	    -gN start game #N
 *	    -b bases (specify # bases per side)
 *	    -s stars (specify # stars)
 *	    -p planets (specify # planets)
 *	    -r rows in galaxy
 *	    -c columns in galaxy
 */
init(ac, av, nstars, nbases, nplanets)
int ac;
char *av[];
int *nstars;
int *nbases;
int *nplanets;
{
	int c, errflg;
	struct stat statbuf;
	char *progname;

	*nstars = *nbases = *nplanets = -1;	/* take game defaults */
	Mrows = DFLT_MROWS;
	Mcols = DFLT_MCOLS;
	Fastgame = 0;
	Gamenum = 1;	/* default game # */
	errflg = 0;
	progname = av[0];
	while ((c = getopt(ac, av, "fg:s:b:p:r:c:")) != EOF) {
		switch(c) {
		case 'f':	/* fastgame enabled */
			Fastgame = 1;
			break;

		case 'g':	/* game number */
			if ((Gamenum = atoi(optarg)) <= 0 ||
				Gamenum > 9) {
				(void)fprintf(stderr,
						"game number range: 1-9\n");
				Uerror = UEUSAGE;
				return(-1);
			}
			break;

		case 's':	/* number of stars */
			*nstars = atoi(optarg);
			break;

		case 'b':	/* number of bases per side */
			*nbases = atoi(optarg);
			break;

		case 'p':	/* number of planets */
			*nplanets = atoi(optarg);
			break;

		case 'r':	/* number of rows in galaxy */
			Mrows = atoi(optarg);
			if (Mrows > 999)
				Mrows = 999;
			if (Mrows < 2)
				Mrows = 2;
			break;

		case 'c':	/* number of columns in galaxy */
			Mcols = atoi(optarg);
			if (Mcols > 999)
				Mcols = 999;
			if (Mcols < 2)
				Mcols = 2;
			break;

		case '?':	/* error */
		default:
			errflg++;
			break;
		}
	}

	if (errflg) {
		(void)fprintf(stderr
		, "usage: %s [-f] [-gN] [-s stars] [-b bases] [-p planets]\
 [-r rows] [-c columns]\n", progname);
		Uerror = UEUSAGE;
		return(-1);
	}

	initrnd();

	msgempty = TRUE;	/* no messages buffered for all_cmd() */

	/* if input FIFO exists, fail */
	(void)sprintf(Ififonam, "/tmp/UnIwAr%d", Gamenum);
	if (stat(Ififonam, &statbuf) != -1) {
		(void)fprintf(stderr, "%s exists (game %d in progress)\n",
				Ififonam, Gamenum);
		Uerror = UEUSAGE;
		return(-1);
	} else {
		(void)umask(0);
		if (mknod(Ififonam, S_IFIFO | 0777, 0) != 0) {
			(void)fprintf(stderr, "Cannot create FIFO %s\n",
							Ififonam);
			Uerror = UEUNIX;
			return(-1);
		}
		if ((Ifd = open(Ififonam, O_RDONLY | O_NDELAY)) == -1) {
			(void)fprintf(stderr, "Cannot open FIFO %s, errno=%d\n"
					, Ififonam, errno);
			Uerror = UEUNIX;
			return(-1);
		}
	}

	nocleanup = 1;	/* no cleanup on interrupt, just quit */

	/* we die neatly if we get a kill -15 (i.e., SIGTERM) */
	(void)signal(SIGTERM, enditall);
	(void)signal(SIGINT, SIG_IGN);
	(void)signal(SIGQUIT, SIG_IGN);
	(void)signal(SIGHUP, SIG_IGN);
	(void)signal(SIGPIPE, SIG_IGN);	/* want errors from write */

	/* initialize other structures that never change */
	(void)sprintf(Lockfile, "%s/lock", GAMEDIR);
	(void)sprintf(Scorefile, "%s/score", GAMEDIR);
	Dpid = getpid();	/* used for lockfile */

	return(0);
}
