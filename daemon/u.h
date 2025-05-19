/* macros */
#define OBJMASK	07777
#define OBJTYPE(x)	((x & ~OBJMASK)>>12)
#define OBJINDX(x)	(x&OBJMASK)
#define outbuf		omsgp.mtext

/* ship's devices (NDEVS is the number of devices) */
#define	DWARP	0
#define	DIMPULS	1
#define	DTUBES	2
#define	DPHASER	3
#define	DSHIELD	4
#define	DCOMPU	5
#define	DLIFE	6
#define	DRADIO	7
/*	#define	DTRACT	8	-- not implemented */
#define NDEVS	8

/* info and limits about ships' devices */
struct devinfo {
	char *name;
	long maxdmg;
	short hitprob;
};

/* object types */
#define	EMPTY	0
#define	STAR	1
#define	SHIP	2
#define	PORT	3	/* Base is a 5-built port, neutral is -1 built */
#define	FPORT	3	/* Federation port */
#define	EPORT	4	/* Empire port */
#define	NPORT	5	/* neutral port */

/* command types */
#define	BADTOK	-1
#define	BUILD	0
#define	CAPTURE	1
#define	DOCK	2
#define	IMPULSE	3
#define	MOVE	4
#define	PHASER	5
#define	REPAIR	6
#define	TORPEDO	7
#define	TELL	8
#define	QUIT	9
#define	ACTIVATE 10
#define	SCAN 11
#define	SHIELD 12
#define	STATUS 13
#define	DAMAGE 14
#define SET 15
#define LIST 16
#define USERS 17
#define SUMMARY 18
#define BASES 19
#define PLANETS 20
#define TARGETS 21
#define POINTS 22
#define AMBIGTOK 100

/*
 * Keywords (for LIST and TELL commands).
 * Also for BAses, SUmmary, PLanets, TArgets commands.
 * Not all keywords work with all commands.
 */
#define	KCLOSEST 1			/* closest */
#define	KSHIPS 2			/* ships */
#define	KBASES 4			/* bases */
#define	KPLANETS 010			/* planets */
#define	KPORTS (KBASES|KPLANETS)	/* ports (both planets and bases) */
#define	KFEDERATION 020			/* federation */
#define	KHUMAN KFEDERATION		/* human (same as federation) */
#define	KEMPIRE 040			/* empire */
#define	KKLINGON KEMPIRE		/* klingon (same as empire) */
#define	KFRIENDLY 0100			/* friendly */
#define	KENEMY 0200			/* enemy */
#define	KTARGETS 0400			/* targets (enemies in range) */
#define	KNEUTRAL 01000			/* neutral */
#define	KCAPTURED (KPLANETS|KFRIENDLY)	/* captured planets */
#define	KALL (KFEDERATION|KEMPIRE)	/* all (both sides) */
#define	KSUMMARY 02000			/* count -- don't list locations */
#define	KNOOUTRNG 04000			/* don't list enemies out of range */
#define NOUNS	(KSHIPS|KPORTS)		/* objects */
#define ADJS (KALL|KNEUTRAL|KFRIENDLY|KENEMY)	/* adjectives */

/*
 * ways to score (or lose) points.
 */
#define NSCORE	8	/* number of ways to score plus total */
#define SDMGENE	0	/* damage to enemies (decremented when hit self) */
#define SDESENE	1	/* destroyed enemies */
#define SDMGBAS	2	/* damage to bases */
#define SCAPL	3	/* captured planets */
#define SBASBLT	4	/* bases built */
#define SDESSTR	5	/* stars destroyed */
#define SDESPL	6	/* destroyed planets */
#define	STOTAL	7	/* total points */

/*
 * point values of certain actions
 */
#define PODESENE 500	/* points for destroying an enemy ship */
#define	POCAPL	100	/* points for capturing a planet */
#define	POBASBLT 1000	/* points for building a base */
#define PODESSTR -50	/* points for destroying a star */
#define PODESPL	-100	/* points for destroying a planet */
/*
 * message types from daemon
 */
#define	UmSCAN	1	/* update scan window (see comment for doscan()) */
#define	UmSTATUS 2	/* Null terminated string for status window */
#define	UmTEXT	3	/* Null terminated string for text window */
#define	UmPDIED	4	/* Player died due to poor play :-) */
#define	UmCODE	5	/* encoded info for text window */
#define	UmLIST	6	/* encoded info for text window need paging */
#define	UmINIT 7	/* Daemon is initializing player */
#define	UmSLEEP 8	/* Player must wait (take time) */
#define	UmPOINT 9	/* encoded info for point printing */
#define	UmROWCOL 10	/* row and col dimensions of galaxy follow */

/*
 * daemon error types
 */
#define UEUNIX	1	/* UNIX error */
#define UEINTL	2	/* Internal error (bug) */
#define UEBADTOK 3	/* Bad command parsed */
#define UEAMBIG 4	/* Ambiguous command parsed */
#define UENOTPG 5	/* Can't do this command in pregame */
#define	UETOOFAR 6	/* Maximum warp MRANGE */
#define	UECOLLIDE 7	/* "Collision awerted, keptin!" */
#define	UELEVGAL 8	/* Can't leave the galaxy */
#define	UECROWD 9	/* Universe too crowded to create */
#define	UENONNUM 10	/* Non-numeric coordinates */
#define	UEBADSHIP 11	/* Bad ship name */
#define	UEBIGCMD 12	/* Too many arguments */
#define	UEISEMPTY 13	/* Ship not in game */
#define	UEISDEAD 14	/* Ship dead */
#define	UEHITFR 15	/* Tried to hit friendly object */
#define	UEQUAN 16	/* Illegal quantity (e.g., energy or torps) */
#define	UENOTARG 17	/* Unable to lock on target */
#define UEOUTGAL 18	/* coords out of galaxy */
#define UEUSAGE 19	/* bad args passed when daemon invoked */

/*
 * what happened to an object (also in handler/uniwar.h).
 */
#define	FASER	1	/* somebody hit by phaser. */
#define	TORP	2	/* somebody hit by torpedo. */
#define	NOVA	3	/* somebody hit by nova. */
#define	PDIED	4	/* port died (used to report death to team members). */
#define	NDIED	5	/* star blew up (used when nova happens due to chain
				reaction of novas). */
#define	LISTS	6	/* listing of objects */
#define	PATTK	8	/* port attacked (report attack to team members). */
#define	WHATMASK 017	/* mask off the status of "what" */
#define	RIP	020	/* thing that was hit died */
#define	DISPLAC	040	/* thing that was hit displaced */
#define	OUTOFRNG 0100	/* don't give real coords of object */

struct stats {
	short Ecnt;	/* ships currently in empire */
	short Fcnt;	/* ships currently in federation */
	short totEcnt;	/* ships used by empire in this game */
	short totFcnt;	/* ships used by federation in this game */
	short nport;	/* number of total ports */
	short nplan[3];	/* number of neutrals, emp pls, and fed pls. */
	short nbase[3];	/* number of emp bases and fed bases (nbase[0] unused */
	short baudcnt[2];	/* count players at various baud rates */
	/* more to come */
};

/* baud rate array indices */
#define	BAUD300	0	/* count users at 300 baud (actually everybody less
			   than 1200 baud) */
#define	BAUD1200 1	/* count users at 1200 baud (actually everybody less
			   than 9600 baud but >= 1200 baud). */

struct msgb {
	long	mtype;		/* message type */
	char	mtext[MAXMSG];	/* message text */
};

typedef struct circbuf {
	char	cbuf[MAXPMSG];
	short	ostart;		/* in case need to rollback command */
	short	start;		/* next byte to process in cbuf */
	short	end;		/* last byte to process in cbuf */
} CBUF;

typedef struct starship {
	char	*name;
	struct player *owner;
	short	flags;
} STARSHIP;

typedef struct player {
	char	name[MAXPLNAME];
	int	baudrate;	/* this player's baudrate */
	int	ofd;		/* file descriptor of output FIFO */
	short	pid;		/* player's process id */
	short	uid;		/* player's user id */
	short	rpos;		/* row psn of player */
	short	cpos;		/* col psn of player */
	short	list;		/* free list and used list */
	long	phtime1;	/* last time phaser bank 1 was fired */
	long	phtime2;	/* last time phaser bank 2 was fired */
	long	totime;		/* last time torpedoes were fired */
	short	pending;	/* messages pending for player (< MAXPEND) */
	short	condition;	/* Red, Green, or Yellow */
	short	lifersrv;	/* life support reserves */
	long	damdev[NDEVS];	/* damage to devices * 100 */
	long	score[NSCORE];	/* player's score */
	long	energy;		/* total energy * 100 (not shields) */
	short	torps;		/* torpedoes remaining */
	long	shields;	/* total shield energy * 100 */
	long	damage;		/* total ship damage * 100 */
	long	stardate;	/* how many moves to date */
	long	flags;
	STARSHIP *ship;
	CBUF	plbuf;		/* holds player's stacked commands */
} PLAYER;

typedef struct planet {
	short rpos;
	short cpos;
	short builds;	/* 5 builds == base, -1 builds == neutral */
	/*** unused "wait to chart port" ***
	short fchart;	** feddie wait to chart port (unused) **
	short echart;	** empire wait to chart port (unused) **
	***/
	long atime;	/* time 1st attacked, announce again in PAWAIT secs */
	long flags;
	long shields;	/* base begins with MAXSHIE shields, regenerates
			   MAXSHIE/200 every npl*10 passes through all_cmd() */
} PLANET;

/*
 * player and planet flags (also defined in handler/uniwar.h)
 * don't change EMPIRE and FEDDIE (used as array elements!!)
 */
/* alliance */
#define	EMPIRE	1	/* 1 in this bit means Empire. */
#define	FEDDIE	2	/* 1 in this bit means Federation. */
#define ALIMASK 3	/* mask to determine alliance */
/* player state */
#define	GETSIDE 4	/* player is picking a side */
#define	GETSHIP 010	/* player is picking a ship */
#define	ACTIVE	020	/* 0 in this bit means pregame player */
#define	DIED	040	/* 1 in this bit means player dead (needs cleanup) */
/* ship status */
#define	WANTSOUT 0100	/* 1 in this bit means player just tried to quit */
#define RADIO 0200	/* 1 in this bit means player's radio is on */
#define SHIELDED 0400	/* 1 in this bit means shields are up */
#define DOCKED 01000	/* 1 in this bit means player is docked */
/* player settings */
#define	OSHORT	02000	/* 1 in this bit means player wants short output */
/* if WARN values are changed, Ddoset.c must be changed! */
#define WARNMASK 014000	/* four warning styles: no warning, */
#define WARN1	04000	/* warn with exclamation points in corners, */
#define WARN2	010000	/* warn with exclamation point fences, */
#define WARN3	014000	/* warn with exclamation point area. */
#define ABSOIN	020000	/* 1 in this bit means absolute coords on input */
#define ABSOOUT 040000	/* 1 in this bit means absolute coords on output */
#define RELOUT	0100000	/* 1 in this bit means relative coords on output */
#define SCANON	0200000 /* 1 in this bit means scans should be done */
#define	DOSCAN	0400000	/* see doscan() */
#define	WAITING	01000000	/* waiting for real-time command */
/* has side charted this port (not a player flag) */
#define FKNOWS	0200000	/* 1 in this bit means federation knows this port */
#define EKNOWS	0400000	/* 1 in this bit means empire knows this port */

/* conditions */
#define RED	0	/* in range of enemy weapons */
#define YELLOW	1	/* low on energy */
#define GREEN	2	/* none of the above */
#define PURPLE	3	/* in range of brian :-) */

/* Coordinate types (see getabsrel() and its callers) */
#define	ABSO	2	/* absolute coordinates */
#define	RELA	3	/* relative coordinates */
#define	COMP	4	/* computed coordinates */
#define	BOTH	5	/* both absolute and relative coordinates */

/* what can happen when you try to move or shoot torpedo */
#define HITEDGE	1	/* ran into the edge of the galaxy */
#define HITOBJ	2	/* ran into an object or hit something */
#define TOOFAR	3	/* tried to move or shoot out of range */

/* miscellaneous */
#define TRUE	1
#define FALSE	0

/* score records file structure */
struct scorefile {
	char name[MAXPLNAME];	/* last handle player used */
	long highest;		/* highest score ever attained in single game */
	long total;		/* total score attained */
	long timelast;		/* time last played (according to time(2)) */
	long mission;		/* how many times played */
	short uid;
};
