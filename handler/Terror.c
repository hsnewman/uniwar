#include <curses.h>	/* curses.h automatically includes stdio.h */
#include <errno.h>
#include "config.h"
#include "uniwar.h"
#include <string.h>

extern WINDOW	*textwin;	/* window containing text messages */
extern int	Uerror;		/* stores uniwar error type for error() */
extern char tmpbuf[];	/* used for sprintf prior to textwindow prints */
extern char *Uerrmsg[];

//extern int errno;
//extern char *sys_errlist[];

void textprint();

/*
 * Print error message based on Uerror and possibly errno.
 * Clear Uerror.
 */
void
error(s)
char *s;
{
	if (textwin != NULL) {
		(void)sprintf(tmpbuf, "***%s***\n%s\n", s, Uerrmsg[Uerror]);
		textprint(tmpbuf);
		if (Uerror == UEUNIX) {
			(void)sprintf(tmpbuf, "%s\n", strerror(errno));
			textprint(tmpbuf);
		}
	} else {
		(void)printf("%s%s\n", s, Uerrmsg[Uerror]);
		if (Uerror == UEUNIX)
			(void)printf("%s\n", strerror(errno));
	}
	Uerror = 0;
}

