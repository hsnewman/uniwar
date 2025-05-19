#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern PLAYER ships[];
extern PLANET *planets;
extern short **map;
extern int Mrows, Mcols;
extern char Outbuf[];

void cpsht(), cplng();

/*
 * doscan -- pack info about player's position into a buffer
 * that the player will unpack and display.  Don't support
 * "warn" field full of exclamation points (too slow).
 *
 * After UmSCAN character, here's the format of the thing:
 *
 * ffffYYXXyyxxccyyxxcc ... yyxxccZZ
 *
 * ffff - player flags; especially info about warning (!) chars.
 * YY - row of universe for upper left corner of user's map (short)
 * XX - col of universe for upper left corner of user's map (short)
 * yy - row of universe for printable object (short)
 * xx - col of universe for printable object (short)
 * cc - two character object to go at preceding yyxx
 * ZZ - end of input (-1)
 */
doscan()
{
	short r, c;
	short lastr, lastc;
	short msize;
	register short i, j;
	char *cp;
	short count;	/* count enemy objects */
	short obx;
	short pflags;
	int myside;
	int myknown;
	short hotflg;

	count = 0;
	cp = Outbuf+sizeof(short);
	*cp++ = UmSCAN;

	cplng(CurrPl->flags, &cp);
	r = CurrPl->rpos + RADIUS;
	cpsht(r, &cp);
	c = CurrPl->cpos - RADIUS;
	cpsht(c, &cp);
	msize = 11;	/* 1 for UmSCAN, 4 for ffff, 4 for YYXX, 2 for ZZ */

	if ((lastr = r - 2*RADIUS) < 1)
		lastr = 1;
	if ((lastc = c + 2*RADIUS) > Mcols)
		lastc = Mcols;
	if (r > Mrows)
		r = Mrows;
	if (c < 1)
		c = 1;
	myside = (CurrPl->flags & ALIMASK);
	if (myside == FEDDIE)
		myknown = FKNOWS;
	else
		myknown = EKNOWS;
	for (i = r; i >= lastr; i--)
		for (j = c; j <= lastc; j++)
			if (map[i][j] != EMPTY) {
				cpsht(i, &cp);
				cpsht(j, &cp);
				msize += 6;	/* yyxxcc */
				obx = OBJINDX(map[i][j]);

				switch(OBJTYPE(map[i][j])) {
				case PORT:
					pflags = planets[obx].flags;
					if (planets[obx].builds == 5) {
						if (pflags & FEDDIE) {
							*cp++ = '[';
							*cp++ = ']';
						} else {
							*cp++ = '(';
							*cp++ = ')';
						}
					} else {
						if (pflags & FEDDIE)
							*cp = '+';
						else if (pflags & EMPIRE)
							*cp = '-';
						else
							*cp = ' ';
						cp++;
						*cp++ = '@';
					}
					if ((pflags & ALIMASK) != myside) {
						planets[obx].flags |= myknown;
#ifdef DEBUG
printf("enemy port # %d in range at %d,%d\n", obx, i, j);
printf("flags=%o octal, rpos=%d, cpos=%d, builds=%d\n", planets[obx].flags,
	planets[obx].rpos, planets[obx].cpos, planets[obx].builds); 
#endif
						/* info so warning is printed */
						count++;
						if (CurrPl->flags & WARNMASK) {
							cpsht(i, &cp);
							cpsht(j, &cp);
		cpsht((planets[obx].builds < 2) ? 2 : planets[obx].builds, &cp);
							msize += 6;
						}
					}
					break;

				default:
					/* internal error - never happens */
					/* but if it does, put a star here */
					map[i][j] = STAR;
					/* fall thru */

				case STAR:
					*cp++ = ' ';
					*cp++ = '*';
					break;

				case SHIP:
					if ((ships[obx].flags & ALIMASK) !=
								myside)
						count++;
					if (ships[obx].flags & SHIELDED)
						*cp++ = '<';
					else
						*cp++ = ' ';
					*cp++ = ships[obx].ship->name[0];
					break;
				}
			}
	cpsht((short)(-1), &cp);
	if (CurrPl->flags & SCANON) {
		if (sendcmd(CurrPl, msize) < 0)
			return(-1);
	}
	/*
	 * if SCANON was off, we still needed to do the above
	 * work so planets and bases will get flagged as known,
	 * other players will see him, etc.
	 *
	 * Also, we can't let the player get away without getting
	 * zapped by enemy ports (below):
	 */

	/* Set condition */
	if (count) {	/* enemy in viewing range */
		/* SZAPPROB chance that we get potshot at */
		if (rndrange(0, 99) < SZAPPROB)
			hotflg = 1;
		else
			hotflg = 0;
		count = hotspot(CurrPl, hotflg);
	}
	if (count == 2 || count == 1)	/* enemy in pounding range */
		CurrPl->condition = RED;
	else if (CurrPl->energy < MAXNRGY/5)
		CurrPl->condition = YELLOW;
	else
		CurrPl->condition = GREEN;
	return(1);
}

/*
 * memcpy short i to char buffer *cp.
 */
void
cpsht(i, cp)
short i;
char **cp;
{
	register int j;
	char *p;

	p = (char *)(&i);
	for (j = 0; j < sizeof(short); j++)
		*(*cp)++ = *p++;
}

/*
 * memcpy long i to char buffer *cp.
 */
void
cplng(i, cp)
long i;
char **cp;
{
	register int j;
	char *p;

	p = (char *)(&i);
	for (j = 0; j < sizeof(long); j++)
		*(*cp)++ = *p++;
}
