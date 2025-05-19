#include <stdio.h>
#include <ctype.h>
#include "config.h"
#include "u.h"

extern char *cmdp;
extern STARSHIP fleet[];

/*
 * getship returns a STARSHIP structure for first name in cmdp,
 * and resets cmdp to point to next name.
 * Return NULL if name doesn't match that of any STARSHIP.
 *
 * It is responsibility of calling routine to ensure that cmdp
 * contains a name at all (if it doesn't, NULL will be returned).
 */
STARSHIP *
getship()
{
	register char *savcmdp;
	register int i;

	*cmdp = (char)toupper((int)(*cmdp));

	savcmdp = cmdp;
	for (i = 0; i < MAXPL; i++) {
		if (fleet[i].name[0] == *cmdp && getkey(fleet[i].name) >= 0)
			return(&fleet[i]);
		cmdp = savcmdp;
	}
	return((STARSHIP *)NULL);
}
