#include <stdio.h>
#include "config.h"
#include "u.h"

short **map;
int	Mrows;		/* number of rows in galaxy */
int	Mcols;		/* number of columns in galaxy */
short  *Mapbuf;
PLAYER ships[MAXPL];
PLANET *planets;	/* calloc'd at universe creation time */
short	npl;		/* number of players in game (includes pregame) */
short	npg;		/* number of players in pregame */
short	availship,	/* head of player structure free list */
	usedship;	/* head of used player structure list */
short	msgempty;	/* flag is TRUE when there are no cmds buffered */

int	Ifd;		/* file descriptor for input FIFO */
long	aLiGnMeNt;	/* make sure next variable is aligned */
char	Buffer[MAXMSG];	/* buffer for input FIFO; should be aligned */
char	Outbuf[MAXMSG];	/* buffer for output FIFO; should be aligned */
char	Lockfile[128];	/* lock file name (protects scorefile) */
char	Scorefile[128];	/* scorefile name */
int	Dpid;		/* pid of Daemon process (used for lockfile) */

int baudincr;		/* add to pauses based on slowest baud rate in game */
int Gamenum;		/* Game number (1-9) of this daemon */
int Fastgame;		/* disable sleeps */
char Ififonam[14];	/* input FIFO file name */
int nocleanup;		/* set if cleanup should not be done on interrupt */
struct stats stats;	/* uniwar statistics */
char currcmd[80];	/* commands can't be longer than 80 characters */
char *cmdp;		/* points to next token in currcmd[] */
PLAYER *CurrPl;		/* player whose turn it is */
long allscore[3][NSCORE];	/* FEDDIE and EMPIRE points */
long allstardate[3];		/* stardates used by FEDDIE and EMPIRE */

/* there should be MAXPL/2 of each alliance's ship */
/* we shelve four ships -- max 7 on a side:
 *	{ "Yorktown",	(PLAYER *)NULL,	FEDDIE	},
 *	{ "Vulcan",	(PLAYER *)NULL,	FEDDIE	},
 *	{ "Wolf",	(PLAYER *)NULL,	EMPIRE	},
 *	{ "Panther",	(PLAYER *)NULL,	EMPIRE	},
 */
STARSHIP fleet[] = {
	{ "Excalibur",	(PLAYER *)NULL,	FEDDIE	},
	{ "Farragut",	(PLAYER *)NULL,	FEDDIE	},
	{ "Intrepid",	(PLAYER *)NULL,	FEDDIE	},
	{ "Lexington",	(PLAYER *)NULL,	FEDDIE	},
	{ "Nimitz",	(PLAYER *)NULL,	FEDDIE	},
	{ "Savannah",	(PLAYER *)NULL,	FEDDIE	},
	{ "Trenton",	(PLAYER *)NULL,	FEDDIE	},
	{ "Buzzard",	(PLAYER *)NULL,	EMPIRE	},
	{ "Cobra",	(PLAYER *)NULL,	EMPIRE	},
	{ "Demon",	(PLAYER *)NULL,	EMPIRE	},
	{ "Goblin",	(PLAYER *)NULL,	EMPIRE	},
	{ "Hawk",	(PLAYER *)NULL,	EMPIRE	},
	{ "Jackal",	(PLAYER *)NULL,	EMPIRE	},
	{ "Manta",	(PLAYER *)NULL,	EMPIRE	},
};

int	Uerror;		/* stores uniwar error type for error() */

/* maximum warp is really MRANGE -- should be fixed here */
char *Uerrmsg[] = {
	"",
	"Unix error",
	"Internal error (bug)",
	"Bad command",
	"Ambiguous command",
	"Command not available in pregame",
	"Out of range",
	"\"Collision awerted, keptin!\"",
	"Can't leave the galaxy",
	"Universe too crowded",
	"Non-numeric coordinates",
	"Bad ship name",
	"Too many arguments",
	"Ship not in game",
	"Ship dead",
	"Tried to hit friendly object",
	"Illegal quantity",
	"Unable to lock on target",
	"Coordinates out of galaxy",
	"Bad args passed when daemon invoked",
};

struct devinfo devinfo[] = {
	{ "warp engines",	60000,	25	},	/* warp */
	{ "impulse engines",	90000,	45	},	/* impulse */
	{ "torpedo tubes",	30000,	55	},	/* tubes */
	{ "phasers",		30000,	65	},	/* phaser */
	{ "shield control",	30000,	80	},	/* shield */
	{ "computer",		30000,	85	},	/* computer */
	{ "life support",	30000,	90	},	/* life */
	{ "sub-space radio",	30000,	100	},	/* radio */
};
