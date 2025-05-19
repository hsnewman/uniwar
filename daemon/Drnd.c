long time();
void srand();

/*
 * Initialize random number generator.
 */
void
initrnd()
{
	srand((unsigned)time(0L));
}

/*
 * return a pseudo random integer in the range 0 to 2^15 - 1
 */
rnd()
{
	return(rand());
}

/*
 * return a pseudo random integer between a and b, inclusive.
 */
rndrange(a, b)
register int a, b;
{
	register int t;

	if (a > b) {
		t = a;
		a = b;
		b = t;
	}
	t = rnd() % (b-a+1);
	return(t+a);
}
