void enditall(), error();

/*
 * main for the uniwar daemon.  Sending a kill -1 should
 * exit cleanly (kill all players, remove ipc stuff, and
 * exit).
 */
main(ac, av)
int ac;
char *av[];
{
	int nstars, nbases, nplanets;

	/* bring up message queues and catch signals */
	if (init(ac, av, &nstars, &nbases, &nplanets) < 0)
		enditall(0);

	/* create the universe */
	if (universe(nstars, nbases, nplanets) < 0)
		enditall(0);

	/* mediate the game */
	if (play() < 0) {
		error(av[0]);
		enditall(0);
	}

	/* kill all the players, free data structures */
	(void)cleanup();
	enditall(0);
}
