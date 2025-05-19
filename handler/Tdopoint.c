#include <curses.h>	/* curses.h automatically includes stdio.h */
#include "config.h"
#include "uniwar.h"

extern char *fleet[];
extern short linenum;
extern WINDOW *textwin;
extern WINDOW *cmdwin;

char *strcat();
void textprint(), getmsgsht(), getmsglng();

char *ptypes[] = {
	"enemy hit",
	"enemy RIP",
	"base hit ",
	"plnt cap ",
	"base blt ",
	"star RIP ",
	"plnt RIP ",
	"  TOTAL: ",
};

/*
 * Interpret coded text as info about points.
 * First short is ship index, rest are score info.
 */
void
dopoint()
{
	char tbuf[64];
	long pscr, fscr, escr;	/* points earned by player and sides */
	long pstd, fstd, estd;	/* stardates used by player and sides */
	short totFcnt, totEcnt;	/* ships used by sides */
	register int i;
	short shx;

	/* read the header info */
	getmsgsht(&shx);	/* get ship index */

	/* move curs to top of textwin after --more-- prompt */
	wmove(textwin, linenum, 0);
	wattron(textwin, A_REVERSE|A_STANDOUT);
	waddstr(textwin, "--more--");
	wattroff(textwin, A_REVERSE|A_STANDOUT);
	wrefresh(textwin);
	nodelay(cmdwin, FALSE);
	(void)getch();
	nodelay(cmdwin, TRUE);
	linenum = 0;
	wmove(textwin, linenum, 0);
	wclrtobot(textwin);
	(void)sprintf(tbuf
		, "%16s    Fed    Emp\n------------------------------\n",
								fleet[shx]);
	textprint(tbuf);
	for (i = 0; i < NSCORE; i++) {
		getmsglng(&pscr);
		getmsglng(&fscr);
		getmsglng(&escr);
		(void)sprintf(tbuf, "%s%7d%7d%7d\n", ptypes[i], pscr, fscr
								, escr);
		textprint(tbuf);
	}
	textprint("-----\n");

	/* total ships */
	getmsgsht(&totFcnt);
	getmsgsht(&totEcnt);
	(void)sprintf(tbuf, "ships used      %7d%7d\n", totFcnt, totEcnt);
	textprint(tbuf);
	if (totFcnt == 0)
		totFcnt = 1;
	if (totEcnt == 0)
		totEcnt = 1;
	(void)sprintf(tbuf, "score/player    %7d%7d\n", fscr/totFcnt
							, escr/totEcnt);
	textprint(tbuf);

	/* total stardates */
	getmsglng(&pstd);
	if (pstd == 0)
		pstd = 1;
	getmsglng(&fstd);
	if (fstd == 0)
		fstd = 1;
	getmsglng(&estd);
	if (estd == 0)
		estd = 1;
	(void)sprintf(tbuf, "score/SD %7d%7d%7d\n", pscr/pstd, fscr/fstd
							, escr/estd);
	textprint(tbuf);
}
