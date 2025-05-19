extern char *cmdp;

/*
 * getkey - return 0 if 'str' is partially matched by the token
 * pointed to by cmdp.  Set cmdp to the next token.
 */
getkey(str)
char *str;
{
	/* don't get fancy with the ++'s */
	while(*str == *cmdp && *str != '\0') {
		str++;
		cmdp++;
	}
	if (*cmdp == ' ') {
		cmdp++;			/* go to next token */
		return(0);		/* match */
	} else if (*cmdp == '\0')	/* no tokens left */
		return(0);		/* match */
	return(-1);			/* no match */
}

