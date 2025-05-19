/*
 * Play's job is to try to keep up with the incoming commands.
 * The input FIFO is read completely as often as possible
 * (actually, once for each pass through the list of players),
 * individual player's commands being placed on their individual
 * message buffers.
 *
 * We loop through the players (all_cmd) -- if a player has a command
 * waiting, we execute it, then move on to the next player.
 *
 * Certain commands "take time" (e.g., building, moving), so
 * players can be put into a "wait state".  This means that
 * we might skip a player even if s/he has a command pending.
 * The player process sends a special msg (RESPCHR) when it's
 * through waiting.
 *
 * Most commands require a reply to the user -- some require
 * messages to be sent to more than one player.  We hope the
 * FIFO mechanism can handle it -- msgs didn't seem to work
 * when the going got tough.
 */
play()
{
	for (;;) {
		/*
		 * read the message queue -- if player sends
		 * EOF, kill him.
		 */
		if (getmsgs() < 0)
			return(-1);	/* fatal error */

		/* process player commands */
		switch (all_cmd()) {
			case -1:
				return(-1);	/* fatal error */

			case 0:
				return(0);	/* game ended */

			default:
				break;		/* game continues */
		}
	}
}
