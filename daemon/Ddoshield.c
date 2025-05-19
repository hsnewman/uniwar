#include <stdio.h>
#include "config.h"
#include "u.h"

extern char *cmdp;
extern PLAYER *CurrPl;
extern int Uerror;

void error();
char *telldmg();
void needscan();

/*
 * doshield - do something to shields:
 *	shield up		-- raise shields
 *	shield down		-- lower shields
 *	shield transfer x	-- transfer x from energy to shields
 *				   if x is negative, tranfer from shields.
 */
doshield()
{
	int tran, ret;
	long dam;
	char tbuf[64];

	switch((int)(*cmdp)) {
	case 'u':
		if (getkey("up") >= 0) {	/* raise them */
			/* damaged() returns 0 if device usable */
			ret = damaged(DSHIELD, CurrPl, &dam);
			if (ret)
				return(pmsg(UmTEXT, CurrPl
					, telldmg(DSHIELD, CurrPl), -1));
			if ((CurrPl->flags & SHIELDED) == 0) {
				CurrPl->flags |= SHIELDED;
				needscan(CurrPl->rpos, CurrPl->cpos);
			}
			return(decnrgy(CurrPl, (long)SHRAISE));
		}
		break;

	case 'd':
		if (getkey("down") >= 0) {	/* lower them */
			if (CurrPl->flags & SHIELDED) {
				CurrPl->flags &= ~SHIELDED;
				needscan(CurrPl->rpos, CurrPl->cpos);
			}
		}
		break;

	case 't':
		if (getkey("transfer") >= 0) {	/* transfer them */
			switch(sscanf(cmdp, "%d %1s", &tran, tbuf)) {
			case EOF:
			case 0:
				Uerror = UENONNUM;
				error("");
				return(1);
			case 1:
				break;
			
			default:
				Uerror = UEBIGCMD;
				error("");
				return(1);
			}
			tran *= 100;
			if (tran < 0) {	/* from shields */
				tran = -tran;
				tran = MIN(tran, CurrPl->shields);
				tran = MIN(tran, MAXNRGY - CurrPl->energy);
				CurrPl->shields -= tran;
				/*
				if (CurrPl->shields == 0)
					CurrPl->flags &= ~SHIELDED;
				 */
				CurrPl->energy += tran;
			} else {	/* to shields */
				tran = MIN(tran, CurrPl->energy);
				tran = MIN(tran, MAXSHIE - CurrPl->shields);
				if (tran == CurrPl->energy) {
					if (pmsg(UmTEXT, CurrPl,
		"Science Officer:\n\"That would be suicide!\"\n", -1) == -1)
						return(-1);
				} else {
					CurrPl->energy -= tran;
					CurrPl->shields += tran;
				}
			}
		}
		break;

	default:
		Uerror = UEBADTOK;
		error("args invalid");
		break;
	}
	return(1);
}
