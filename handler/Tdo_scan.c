#include <curses.h>	/* curses.h automatically includes stdio.h */
#include "config.h"
#include "uniwar.h"

extern WINDOW	*mapwin;	/* window containing map (short-range scan) */

extern short	ycorner,	/* col in universe for upper left of mapwin */
		xcorner;	/* col in universe for upper left of mapwin */
extern int	Mrows,		/* number of rows in galaxy */
		Mcols;		/* number of columns in galaxy */

#ifdef DEBUG
extern char tmpbuf[];	/* used for sprintf prior to textwindow prints */
#endif

void getmsgsht(), getmsglng(), textprint(), warnmarks();
#define NWARN 30	/* can't warn about more than 30 objects */
struct w {
	short r;
	short c;
	short range;
};

/*
 * do_scan -- update mapwin from scan info provided in msg
 * from daemon.
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
 *
 * In the default map window (designed for 24x80 screen):
 *	- rows 0 and 20 for column (x) coords
 *	- rows 1-19 for the universe (ship scan range (RADIUS) is 9)
 *	- cols 0-3 and 42-45 are for row (y) coords
 *	- cols 4-41 for universe (2 spaces per coord)
 */
void
do_scan()
{
	register short	r, c;
	short	x, y;
	int	c1, c2;
	short	trow, brow, lcol, rcol;
	short	front, back;
	register short count;
	long	pflags;
	struct w warn[NWARN];	/* save coords that need warning chars */
	struct w *twarn;
	int nwarn;

	/* take care of coordinate labels */
	getmsglng(&pflags);
	getmsgsht(&y);
	getmsgsht(&x);
	if (y != ycorner) {	/* redo the y's */
		ycorner = y;
		for (r = 1; r < MAPROWS-1; r++) {
			if (y > 0 && y <= Mrows && !(y%2)) {
				mvwprintw(mapwin, r, 0, "%3d", y);
				mvwprintw(mapwin, r, MAPCOLS-3, "%3d", y);
			} else {
				mvwaddstr(mapwin, r, 0, "   ");
				mvwaddstr(mapwin, r, MAPCOLS-3, "   ");
			}
			y--;
		}
	}
	if (x != xcorner) {	/* redo the x's */
		xcorner = x;
		for (c = 3; c < MAPCOLS-6; c += 2) {
			if (x > 0 && x <= Mcols && !(x%2)) {
				mvwprintw(mapwin, 0, c, "%3d", x);
				mvwprintw(mapwin, MAPROWS-1, c, "%3d", x);
			} else {
				mvwaddstr(mapwin, 0, c+1, "  ");
				mvwaddstr(mapwin, MAPROWS-1, c+1, "  ");
			}
			x++;
		}
	}

	/*
	 * Now do the grid.  We assume that the daemon is feeding
	 * us objects left to right, top to bottom.
	 */
	getmsgsht(&y);	/* y can't be -1 since I know I am a valid object */
	getmsgsht(&x);
/*
#ifdef DEBUG
	(void)sprintf(tmpbuf, "next obj at (%d,%d)\n", y, x);
	textprint(tmpbuf);
#endif
*/
	c1 = (short)getmsgch();
	c2 = (short)getmsgch();

	nwarn = 0;	/* number of objects to warn about */

	for (r = ycorner; r > ycorner - (MAPROWS-2); r--) {
		wmove(mapwin, ycorner-r+1, 4);
		c = xcorner;
putdots:
		if (r != y) {	/* there are no more objects on this row */
			for (; c < xcorner + (MAPCOLS-8)/2; c++)
				waddstr(mapwin, " .");
			continue;
		}
		/* there is at least one object on this map */
		for (; c < xcorner + (MAPCOLS-8)/2; c++) {
			if (c != x || x == -1) {
				waddstr(mapwin, " .");
				continue;
			} 

			/* there is an object here. */
			waddch(mapwin, (char)c1);
			waddch(mapwin, (char)c2);
/*
#ifdef DEBUG
	wrefresh(mapwin);
#endif
*/

			/*
			 * Get a new yyxxcc.  (Place dots until we
			 * get to yyxx, then place cc and get a new
			 * yyxxcc.)
			 */
newcoord:
			getmsgsht(&y);
			if (y == -1) {
				x = -1;
				continue;	/* no more objects at all */
			}
			getmsgsht(&x);

			/*
			 * Need to print warn marks.
			 */
			if (x == c && y == r) {
				if (nwarn + 1 == NWARN)	/* ignore warning */
					goto newcoord;
				twarn = &warn[nwarn];
				twarn->r = r;
				twarn->c = c;
				getmsgsht(&twarn->range);
				++nwarn;
/*
#ifdef DEBUG
(void)sprintf(tmpbuf, "enemy obj at (%d,%d)\n\trow=%d, col=%d\n\n", y, x
				, ycorner-r+1, 4+(c-xcorner)*2);
textprint(tmpbuf);
#endif
*/
				goto newcoord;
			} else {
				c1 = (short)getmsgch();
				c2 = (short)getmsgch();
/*
#ifdef DEBUG
	(void)sprintf(tmpbuf, "next obj at (%d,%d)\n", y, x);
	textprint(tmpbuf);
#endif
*/
			}
			c++;
			goto putdots;
		}
	}

	/* fill in the borders if necessary */
	trow = brow = lcol = rcol = -1;
	if (ycorner > Mrows)		/* there is a top horiz barrier */
		trow = ycorner-Mrows;	/* print barrier on row trow */
	if (ycorner < (2*RADIUS)+1)	/* there is a bottom horiz barrier */
		brow = ycorner+1;	/* print barrier on row brow */
	if (xcorner < 1)		/* there is a left vertical barrier */
		lcol = 4 - 2*xcorner;	/* print barrier on col lcol */
	if (xcorner > Mcols-(2*RADIUS))	/* there is a right vertical barrier */
		/* 4 + 2*((2*RADIUS)+2) - (2*(xcorner-(Mrows-(2*RADIUS)+1))-4 */
		rcol = 2*Mcols + 6 - 2*xcorner;	/* barrier on col rcol */

	/* print top barrier */
	if (trow != -1) {
		front = (lcol != -1) ? lcol : 4;
		back = (rcol != -1) ? rcol : 4*RADIUS + 4;
		count = (back-front)/2 + 1;
		wmove(mapwin, trow, front);
		while(count--)
			waddstr(mapwin, "--");
	}
	/* print bottom barrier */
	if (brow != -1) {
		front = (lcol != -1) ? lcol : 4;
		back = (rcol != -1) ? rcol : 4*RADIUS + 4;
		count = (back-front)/2 + 1;
		wmove(mapwin, brow, front);
		while(count--)
			waddstr(mapwin, "--");
	}
	/* print left barrier */
	if (lcol != -1) {
		front = (trow != -1) ? trow : 1;
		back = (brow != -1) ? brow : 2*RADIUS + 1;
		for (r = front; r <= back; r++)
			mvwaddstr(mapwin, r, lcol, " |");
	}
	/* print right barrier */
	if (rcol != -1) {
/*
#ifdef DEBUG
	(void)sprintf(tmpbuf, "rcol is %d\n", rcol);
	textprint(tmpbuf);
#endif
*/
		front = (trow != -1) ? trow : 1;
		back = (brow != -1) ? brow : 2*RADIUS + 1;
		for (r = front; r <= back; r++)
			mvwaddstr(mapwin, r, rcol, " |");
		if (trow != -1)
			mvwaddch(mapwin, trow, rcol, '-');
		if (brow != -1)
			mvwaddch(mapwin, brow, rcol, '-');
	}
	/* print '!' marks around enemy objects */
	warnmarks(pflags, warn, nwarn);
}

#define putwarn(r,c) if(mvwinch(mapwin,r,c)=='.')mvwaddch(mapwin,r,c,WARNCHR)

/*
 * warnmarks - print '!' marks (WARNCHR) around enemy objects
 * locations saved in warn[] array.
 */
void
warnmarks(pflags, warn, nwarn)
long pflags;
struct w warn[];
int nwarn;
{
	struct w *tw, *warnL;
	short trow, brow, lcol, rcol;
	short r, c;

	warnL = &warn[nwarn];
	for (tw = warn; tw < warnL; tw++) {
		/* upper row */
		trow = MAX(ycorner - (tw->r + tw->range) + 1, 1);
		if (ycorner > Mrows)	/* keep marks inside grid */
			trow = MAX(trow, ycorner - Mrows + 1);

		/* bottom row */
		brow = MIN(ycorner - (tw->r - tw->range) + 1, MAPROWS-2);
		if (ycorner < (2*RADIUS)+1)	/* keep marks inside grid */
			brow = MIN(brow, ycorner);

		/* left column */
		lcol = MAX(((tw->c - tw->range) - xcorner) * 2 + 5, 5);
		if (xcorner < 1)
			lcol = MAX(lcol, 7 - 2*xcorner);

		/* right column */
		rcol = MIN(((tw->c + tw->range) - xcorner) * 2 + 5
			, 2*((2*RADIUS)+2) + 1);
		if (xcorner > Mcols-(2*RADIUS))
			rcol = MIN(rcol, 2*Mcols + 5 - 2*xcorner);

		switch(pflags & WARNMASK) {
		case WARN1:	/* corners only */
			putwarn(trow, lcol);
			putwarn(trow, rcol);
			putwarn(brow, lcol);
			putwarn(brow, rcol);
			break;

		case WARN2:	/* walls only */
			/* top and bottom */
			for (c = lcol; c <= rcol; c += 2) {
				putwarn(trow, c);
				putwarn(brow, c);
			}
			/* sides */
			for (r = trow+1; r < brow; r++) {
				putwarn(r, lcol);
				putwarn(r, rcol);
			}
			break;

		case WARN3:	/* entire area */
			for (r = trow; r <= brow; r++)
				for (c = lcol; c <= rcol; c += 2)
					putwarn(r, c);
			break;

		default:
			break;
		}
	}
}

