#include <stdio.h>
#include "config.h"
#include "u.h"
#include "string.h"
#include <errno.h>

extern PLAYER *CurrPl;
extern int Uerror;
extern char *Uerrmsg[];

extern char *sys_errlist[];
//extern int errno;

/*
 * Print error message based on Uerror and possibly errno.
 * Clear Uerror.
 */
void
error(s)
register char *s;
{
	char tbuf[MAXMSG];
	register char *cp;

	cp = tbuf;
	if (strlen(s) > 0) {
		(void)sprintf(cp, "%s:\n", s);
		cp = &tbuf[strlen(tbuf)];
	}
	(void)sprintf(cp, "%s%s\n", Uerrmsg[Uerror]
			, (Uerror == UEUNIX) ? strerror(errno) : "");
	Uerror = 0;
	/* naughty to (void) but I'll fix it someday */
	(void)pmsg(UmTEXT, CurrPl, tbuf, -1);
#ifdef DEBUG
	(void)fprintf(stderr, tbuf);
	(void)fflush(stderr);
#endif
}
