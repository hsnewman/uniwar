#include <signal.h>
#include "config.h"
#include "uniwar.h"

extern int	Uerror;		/* stores uniwar error type for error() */

void enditall(), inthandler();

/*
 * Catch signal 'sig'.
 */
setsig(sig)
register int sig;
{
	int (*oldsig)();

	switch(sig) {
	case SIGHUP:
		oldsig = signal(sig, enditall);
		break;

	case SIGINT:
		oldsig = signal(sig, inthandler);
		break;

	case SIGQUIT:
		oldsig = signal(sig, inthandler);
		break;

	case SIGTERM:
		oldsig = signal(sig, enditall);
		break;

	default:
		oldsig = signal(sig, enditall);
		break;
	}

	if (oldsig == (int(*)())(-1)) {
		Uerror = UEUNIX;
		return(-1);
	}

	/* in case signal already ignored from outside */
	if (oldsig == (int(*)())(SIG_IGN)) {
		(void)signal(sig, (int(*)())(SIG_IGN));
	}
	return(0);
}

