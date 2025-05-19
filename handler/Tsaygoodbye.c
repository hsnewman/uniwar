#include "config.h"
#include "uniwar.h"

extern char Outbuf[];

/*
 * Send message to daemon saying "kill me -- I aborted".
 */
void
saygoodbye()
{
	Outbuf[2*sizeof(short)] = GOODBYE;	/* end of file (quit) */
	(void)write_it((short)1);
}

