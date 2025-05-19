#include <stdio.h>
#include "config.h"
#include "u.h"

extern long allscore[][NSCORE];
extern PLAYER *CurrPl;

/*
 * scoreit -- somebody or some side scored points.
 */
void
scoreit(what, sflag, flag, hit, pl1, pl2, pt)
int what;
int sflag;
int flag;
long hit;
PLAYER *pl1, *pl2;
PLANET *pt;
{
	int myside;

	hit /= 100;	/* points not multiplied by 100 ! */
	switch(sflag) {
	case SDESENE:	/* destroyed enemies */
		hit = PODESENE;
		/* fall thru */

	case SDMGENE:	/* damage to enemies (decr when hit self) */
	case SDMGBAS:	/* damage to bases */
		if (pl1 != (PLAYER *)NULL)
			myside = pl1->flags & ALIMASK;
		switch(flag) {
		case 1:	/* pl1 hit pl2 */
		case 2:	/* pl1 hit pt */
			pl1->score[sflag] += hit;
			pl1->score[STOTAL] += hit;
			allscore[myside][sflag] += hit;
			allscore[myside][STOTAL] += hit;
			break;
		case 3:	/* pt hit pl2 */
			/* if pl2 on same side as CurrPl, hurt own side! */
			myside = CurrPl->flags & ALIMASK;
			if ((pl2->flags & ALIMASK) == myside) {
				CurrPl->score[sflag] -= hit;
				CurrPl->score[STOTAL] -= hit;
				allscore[myside][sflag] -= hit;
				allscore[myside][STOTAL] -= hit;
			} else {
				/* credit to the guy that blew up the star */
				if ((what & WHATMASK) == NOVA) {
					CurrPl->score[sflag] += hit;
					CurrPl->score[STOTAL] += hit;
				}
				allscore[myside][sflag] += hit;
				allscore[myside][STOTAL] += hit;
			}
			break;
		case 4:	/* pt hit by nova (CurrPl scores) */
			/* if pt on same side as CurrPl, hurt own side! */
			myside = CurrPl->flags & ALIMASK;
			if ((pt->flags & ALIMASK) == myside) {
				CurrPl->score[sflag] -= hit;
				CurrPl->score[STOTAL] -= hit;
				allscore[myside][sflag] -= hit;
				allscore[myside][STOTAL] -= hit;
			} else {
				/* credit to the guy that blew up the star */
				if ((what & WHATMASK) == NOVA) {
					CurrPl->score[sflag] += hit;
					CurrPl->score[STOTAL] += hit;
				}
				allscore[myside][sflag] += hit;
				allscore[myside][STOTAL] += hit;
			}
			break;
		default:
			break;
		}
		return;

	case SCAPL:	/* captured planets */
		hit = POCAPL;
		break;

	case SBASBLT:	/* bases built */
		hit = POBASBLT;
		break;

	case SDESSTR:	/* stars destroyed */
		hit = PODESSTR;
		break;

	case SDESPL:	/* planets destroyed */
		hit = PODESPL;
		break;

	default:
		break;
	}
	CurrPl->score[sflag] += hit;
	CurrPl->score[STOTAL] += hit;
	allscore[CurrPl->flags & ALIMASK][sflag] += hit;
	allscore[CurrPl->flags & ALIMASK][STOTAL] += hit;
}
