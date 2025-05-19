#define MAX(x,y)	((x)>(y)?(x):(y))
#define MIN(x,y)	((x)<(y)?(x):(y))

#define GAMEDIR	"/units4/wws/junkdir/wwsmisc/src/nuniwar"
#define MAXMSG	1024	/* should be maximum buffering by FIFO on this system */
#define MAXPMSG	256	/* max typeahead for player */
#define MAXPEND	20	/* max messages pending for player */
#define MAXPL	14	/* max players.  If you increase this, you will need to
			   add new ships to u.h and uglobs.c */
#define MAXPG	5	/* max players in pregame */
#define MAXPLNAME 16	/* maximum length of player name */
#define	CMDSEP	'/'	/* command separator (must be same as in uniwar.h) */
#define	ABORT	'\3'	/* player wants to abort stacked commands */
#define	GOODBYE	'\4'	/* player quit and won't take any more messages */
#define	BAUDCHR	'\5'	/* player's baudrate contained in this msg */
#define	RESPCHR	'\6'	/* acknowledgement of msg from daemon */
#define WARNCHR	'!'	/* area is controlled by enemy port */
#define MRANGE	8	/* max move for player */
#define TRANGE	8	/* max range for torpedo */
#define PRANGE	9	/* max range for phaser (should match RADIUS) */
#define VICINITY 15	/* max range for observing battles */
#define MAXBASES 10	/* maximum bases that a side can have */
#define MAXFAILS 100	/* how many failures to find empty map spot before
			   giving up on universe creation */
#define SZAPPROB 25	/* odds that scan will cause a potshot (SZAPPROB/100) */
#define ZAPPROB 30	/* odds that neutral will fire (ZAPPROB/100) */
#define EZAPPROB 60	/* odds that enemy port will fire (EZAPPROB/100) */
#define MOVNRGY	500	/* costs MOVNRGY * dist^2 to move, unshielded */
#define CAPNRGY 5000	/* costs CAPNRGY to capture or build planet */
#define	DEFPHAS	20000	/* default phaser blast * 100 */
#define	MAXPHAS	50000	/* max phaser blast * 100 */
#define	MAXTBRST 3	/* max torps fired per burst */
#define TURNREP	3000	/* repair TURNREP units per damaged device every turn */
#define DEFREP	5000	/* default units to repair per device (REpair cmd) */
#define SHRAISE	10000	/* energy (* 100) to raise shields */
#define PAWAIT 60	/* seconds between "attacked" msgs from bases */
#define MINWAITIME 30	/* seconds to wait after you die before you can rejoin */

/*
 * default seconds for commands that require pauses.
 */
#define	PBUILD	5
#define	PCAPTURE 5
#define	PDOCK	2
#define	PMOVE	1	/* both WARP and IMPULSE */
#define	PPHASER	2
#define	PREPAIR	1	/* every 25 units of repair takes PREPAIR secs */
#define	PTORPEDO 2

/* for docking */
#define MAXNRGY	500000	/* ship energy * 100 */
#define MAXSHIE	400000	/* ship shield energy * 100 */
#define MAXBSHIE 300000	/* base shield energy * 100 */
#define BPCTREGEN (MAXBSHIE/100)  /* base shields regen this amount
				     periodically (see Dlifegoeson.c) */
#define MAXTORP	10	/* ship torpedo capacity */
#define MAXLIFE	5	/* life support reserves. */
#define	MAXDMG	400000	/* death-causing ship damage * 100 */
#define	INCNRGY	MAXNRGY/10	/* ship energy increment during dock */
#define	INCSHIE	MAXSHIE/10	/* ship shield energy increment during dock */
#define	INCTORP	5	/* torpedo increment during dock */
#define	INCDMG	5000	/* ship damage decrement during dock */

/*
 * If either DFLT_MROWS or DFLT_MCOLS is greater than 32767, no guarantees since
 * messages pass universe coordinates in shorts.  Other problems:
 *
 * (1) the map is represented in the daemon as a two-dimensional array
 *	of shorts DFLT_MROWSxDFLT_MCOLS.  If the array is too large, the daemon
 *	may not run.
 * (2) the coordinates that are printed around the map should be no more
 *	that three digits long, else the subwindow placement has to be
 *	recalculated and the doscan() routine's wprint()s must change.
 */
#define DFLT_MROWS	100	/* default rows in universe */
#define DFLT_MCOLS	100	/* default rows in universe */

/* may wish to make NSTARS or MAXPLANETS dependent on Mrows and Mcols */
/* on average create this many stars */
#define NSTARS ((Mrows*Mcols)/50)
/* Min neutral planets in game (default)*/
#define MINPLANETS (((Mrows*Mcols)/1000)+1)
/* Max neutral planets in game (default) */
#define MAXPLANETS (((Mrows*Mcols)/100)+2)
/* actually, you can have up to 1/10 of galaxy full of pls; MAXPLANETS
 * is used as MAX when choosing a random number of starting pls */

/*
 * Constants defining size of subwindows.
 * Here's (roughly) what the screen looks like:
 *
 * --------------------------------------------------------------
 * |    8  10  12  14  16  18      Ship's status info goes from |
 * |    . . . . . . . . . . .      here to                      |
 * | 90 . . . . . . . . . . . 90   |                            |
 * |    . . . . . . . . . . .      V                            |
 * | 88 . . . . . . . . . . . 88   here (5 lines)               |
 * |    . . . . . . . . . . .                                   |
 * | 86 . . . . . . . . . . . 86   Messages get printed from    |
 * |    . . . . . . . . . . .      here (2 lines below status)  |
 * | 84 . . . . . . . . . . . 84   to                           |
 * |    . . . . . . . . . . .      |                            |
 * | 82 . . . . . . . . . . . 82   |                            |
 * |    . . . . . . . . . . .      V                            |
 * |    8  10  12  14  16  18      here (2 lines above cmds)    |
 * |                                                            |
 * | previous command goes on this line                         |
 * |>current command goes on this line                          |
 * --------------------------------------------------------------
 *
 * RADIUS is the player's scanning radius (RADIUS=5 in the above example, but
 * RADIUS=9 by default).  DO NOT make RADIUS greater than 9 unless you have
 * screens larger than 24 rows and 80 columns.  More specifically,
 * you need at least (RADIUS*2)+6 rows (to ensure that both the map and
 * the command subwindow will fit on the screen) and (RADIUS*4)+44 columns
 * (to ensure that the status and text messages subwindows get at
 * least 32 columns -- otherwise daemon's messages look garbled).
 *
 * On the screen, the map subwindow is always in the upper left, and
 * its size is determined from RADIUS.
 *
 * Map goes from row MAPBEGY to row (MAPBEGY+MAPROWS-1) (MAPROWS rows)
 * Map goes from col MAPBEGX to col (MAPBEGX+MAPCOLS-1) (MAPCOLS cols)
 *
 * In the default map window (designed for 24x80 screen):
 *	- rows 0 and 20 for column (x) coords
 *	- rows 1-19 for the universe (ship scan range (RADIUS) is 9)
 *	- cols 0-3 and 42-45 are for row (y) coords
 *	- cols 4-41 for universe (2 spaces per coord)
 *
 * status upper right of screen -- 5 rows, 32 cols
 * text right center of screen -- 16 rows, 32 cols
 * commands bottom of screen -- 2 rows, 79 cols
 */
#define	RADIUS	9	
#define MAPROWS	((2*RADIUS)+3)	/* # rows in map window */
#define MAPCOLS	((4*RADIUS)+10)	/* # cols in map window */
#define MAPBEGY	0	/* first row for map window */
#define MAPBEGX	0	/* first col for map window */
#define STATROWS 5	/* # rows in status window */
#define STATCOLS 32	/* # cols in status window (actually calculate*/
#define STATBEGY 0	/* first row for status window */
#define STATBEGX (MAPCOLS+1)	/* first col for status window */
#define TEXTROWS 16	/* # rows in text window */
#define TEXTCOLS 32	/* # cols in text window */
#define TEXTBEGY 6	/* first row for text window */
#define TEXTBEGX (MAPCOLS+1)	/* first col for text window */
#define CMDROWS	2	/* # rows in command window */
#define CMDCOLS	79	/* # cols in command window */
#define CMDBEGY	(MAPROWS+1)	/* first row for command window */
#define CMDBEGX	0	/* first col for command window */
