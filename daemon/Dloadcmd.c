#include <ctype.h>
#include "config.h"
#include "u.h"

extern char currcmd[];
extern char *cmdp;
extern PLAYER *CurrPl;

/*
 * get cmd from CurrPl's buf to currcmd[], and
 * preprocess it (map to lower case and get rid
 * of extra white space.  If no command is waiting
 * (or all commands present are whitespace),
 * return -1, else return 0.
 *
 * Avoid mapping "set name" names or "tell" msgs to lower case.
 */
loadcmd()
{
	register CBUF *buf;
	register char *cp;
	register short spaceflag;
	/*** need these variables for kludge to allow names and msgs ***
	 *** to contain upper case characters.			     ***
	 ***							     ***/
	short tokcnt;		/* number of tokens mapped to lower case */
	short lcase;		/* first, set it if any upper case are found.
				   later, set to 1 when rest of chars are
				   to be mapped to lower case */
	short sflg, tflg;	/* "set" flag and "tell" flag */
	/*** end of vars needed for kludge ***/

	buf = &CurrPl->plbuf;
	/* for "rolling back" command (phaser or torp wait) */
	buf->ostart = buf->start;

NextCmd:
	/* if no command waiting, return */
	if (buf->start == buf->end)
		return(-1);

	/* get rid of leading whitespace */
	while (isspace((int)buf->cbuf[buf->start])) {
		buf->start++;
		if (buf->start == MAXPMSG)
			buf->start = 0;
	}
	spaceflag = 0;
	cp = currcmd;
	lcase = 0;	/* set to non-zero if we find any upper case */
	while ((*cp = buf->cbuf[buf->start]) != CMDSEP) {
		buf->start++;
		if (buf->start == MAXPMSG)	/* circular buffer wraparound */
			buf->start = 0;
		if (isspace((int)(*cp))) {
			if (!spaceflag && cp != currcmd) {
				/* haven't read a space yet and not just
					starting, then save the space */
				*cp++ = ' ';
				spaceflag = 1;
			}
		} else {	/* not a space */
			if (isupper((int)(*cp)))
				lcase++;
			if (*cp == ';') {	/* need to insert a space */
				if (!spaceflag && cp != currcmd) {
					*cp++ = ' ';
					*cp = ';';
				}
				*++cp = ' ';	/* add blank after semicolon */
				spaceflag = 1;
			} else
				spaceflag = 0;
			++cp;
		}
	}
	buf->start++;	/* get past CMDSEP */
	if (buf->start == MAXPMSG)
		buf->start = 0;

	if (cp == currcmd)	/* didn't get anything */
		goto NextCmd;
	*cp = '\0';

	/*
	 *** Kludge ***
	 * Map to lower case until get to name in "set name" or
	 * message in "tell".
	 */
	cmdp = currcmd;
	tokcnt = 0;		/* number of tokens mapped to lower case */
	sflg = tflg = 0;	/* "set" flag and "tell" flag */
	if (lcase) {	/* there are chars to map to lower case */
		lcase = 0;	/* when 1, rest of chars ==> lower case */
		for (cp = currcmd; *cp; cp++) {
			if (lcase || !isspace((int)(*cp))) {
				if (tflg && *cp == ';')
					break;	/* got to msg part of "tell" */

				/* map to lower case */
				*cp = (char)tolower((int)(*cp));
			} else {
				/* just mapped a token to lower case */
				tokcnt++;
				if (tokcnt == 1) {	/* command */
					if (*cmdp == 's' && getkey("set") >= 0)
						sflg++;
					else if (*cmdp == 't' &&
						getkey("tell") >= 0) {
						tflg++;
						/* map to lower case til ';' */
						lcase++;
					}

				} else if (sflg && *cmdp == 'n' &&
						getkey("name") >= 0)
					break;	/* no more lower case */
				else
					lcase++;	/* rest is lcase */
			}
		}
	}
	/*** end of kludge ***/

#ifdef DEBUG
	printf("fixed command is \"%s\"\n", currcmd);
#endif
	cmdp = currcmd;
	return(0);
}
