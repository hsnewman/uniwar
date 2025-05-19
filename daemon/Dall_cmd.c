#include "config.h"
#include "u.h"

extern PLAYER *CurrPl;
extern PLAYER ships[];
extern short usedship;
extern short msgempty;

void lifegoeson();

/*
 * Loop through the players, both active and pregame, doing
 * one command for each player.  If a player is "waiting",
 * don't look at his/her command list.
 *
 * return -1 on fatal error, 0 on game end, 1 on game continues.
 */
all_cmd()
{
	register short i;
	register short count;	/* count non-empty player cmd buffers */
	register short happen;	/* bump if command is executed */
	int ret;

	count = happen = 0;
	for (i = usedship; i != -1; i = CurrPl->list) {
		CurrPl = &ships[i];
		/*
		 * if player was killed, don't process his/her
		 * messages.  Eventually the player will send
		 * a GOODBYE message, and getmsgs() will
		 * clean out the player.  Dead player processes
		 * are detected in pmsg().
		 */
		if (CurrPl->flags & DIED)
			continue;

		/*
		 * If CurrPl is waiting, continue to next player.
		 *   Don't bump count -- we can't do this player's
		 *   msgs until we get a RESPCHR msg from him/her.
		 *
		 * Else get cmd from CurrPl's buf to currcmd[], and
		 *   preprocess it (map to lower case and get rid
		 *   of extra white space).
		 */
		if (CurrPl->flags & WAITING)
			continue;

		ret = loadcmd();
		if (ret < 0)
			continue;	/* no command to process */

		/*
		 * If there are more commands in the player's
		 * buffer, bump count.
		 */
		if (CurrPl->plbuf.start != CurrPl->plbuf.end)
			count++;

		/*
		 * Execute the command.  Check player's state
		 * from flags to help determine how to interpret
		 * command.
		 */
		switch(do_cmd()) {
			case -1:
				return(-1);	/* fatal error */

			case 0:
				return(0);	/* game ended */

			case 1:
				happen++;	/* somebody did time-
						   consuming cmd */
				break;

			default:
				break;		/* game continues */
		}
	}
	if (count)
		msgempty = FALSE;
	else
		msgempty = TRUE;

	if (happen)	/* at least one player is active */
		lifegoeson();	/* bump system stardate if something happened */

	return(1);
}
