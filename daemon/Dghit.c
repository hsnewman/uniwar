#include "config.h"
#include "u.h"

extern STARSHIP fleet[];

void cpsht(), cplng();

/*
 * ghitr - get hitter info and encode it in txt.
 * 'what' is what happened:
 *	FASER-- somebody hit by phaser.
 *	TORP  -- somebody hit by torpedo.
 *	NOVA  -- somebody hit by nova.	
 *	RIP|FASER-- somebody killed by phaser.
 *	RIP|TORP  -- somebody killed by torpedo.
 *	RIP|NOVA  -- somebody killed by nova.	
 *
 * 'flag' indicates who did what.  Either pl hits another player, pl hits
 * another pt, pt hits another pl, or pt hits another pt depending whether
 * flag is 1, 2, 3, or 4 respectively.
 * 'hit' is the size of the hit.
 *
 * if (what & OUTOFRNG), don't list row and col info, and list
 * hit as 0 so we won't look for more info in Tdocode.c
 */
void
ghitr(flag, pl, pt, hit, what, txt)
int flag;
PLAYER *pl;
PLANET *pt;
long hit;
int what;
char **txt;
{
	long shld;

	switch(flag) {
	case 1:
	case 2:	/* pl is the hitter */
		**txt = (char)(pl->ship - fleet);
		**txt <<= 3;
		**txt |= (char)SHIP;
		(*txt)++;
		if (what & OUTOFRNG) {
			cpsht((short)0, txt);
			cpsht((short)0, txt);
			cplng(0L, txt);
		} else {
			cpsht(pl->rpos, txt);
			cpsht(pl->cpos, txt);
			cplng(hit, txt);
			if ((pl->flags & SHIELDED) == 0)
				shld = -pl->shields;
			else
				shld = pl->shields;
			cplng(shld, txt);
		}
		break;

	case 3:
	case 4:	/* pt is the hitter */
		if ((what & WHATMASK) != NOVA)
			**txt = (char)(pt->builds << 3);

		if ((what & WHATMASK) == NOVA)
			**txt = (char)STAR;
		else if (pt->flags & EMPIRE)
			**txt |= (char)EPORT;
		else if (pt->flags & FEDDIE)
			**txt |= (char)FPORT;
		else
			**txt |= (char)NPORT;
		(*txt)++;
		cpsht(pt->rpos, txt);
		cpsht(pt->cpos, txt);
		cplng(hit, txt);

		/* if it's a nova, pt is a star. */
		/* if we're listing, don't append shields if hit is 0 */
		if ((what & WHATMASK) != NOVA && pt->builds == 5 &&
			(hit || (what & WHATMASK) != LISTS))
			cplng(pt->shields, txt);
		break;

	default:
		break;
	}
}

/*
 * ghite - get hittee info and encode it in txt.
 * 'what' is what happened:
 *	FASER-- somebody hit by phaser.
 *	TORP  -- somebody hit by torpedo.
 *	NOVA  -- somebody hit by nova.	
 *	RIP|FASER-- somebody killed by phaser.
 *	RIP|TORP  -- somebody killed by torpedo.
 *	RIP|NOVA  -- somebody killed by nova.	
 *
 * 'flag' indicates who did what.  Either pl is hit by another player,
 * pt is hit by another pl, pl is hit by another pt, or pt is hit by
 * another pt depending whether flag is 1, 2, 3, or 4 respectively.
 *
 * Set r and c to the absolute location of the hittee.
 */
void
ghite(flag, pl, pt, what, txt, r, c)
int flag;
PLAYER *pl;
PLANET *pt;
int what;
char **txt;
short *r, *c;
{
	long shld;

	switch(flag) {
	case 1:
	case 3:	/* pl is the hittee */
		**txt = (char)(pl->ship - fleet);
		**txt <<= 3;
		**txt |= (char)SHIP;
		(*txt)++;
		*r = pl->rpos;
		*c = pl->cpos;
		if (what & RIP)	/* hittee died -- don't need more info */
			return;
		cpsht(pl->rpos, txt);
		cpsht(pl->cpos, txt);
		if ((pl->flags & SHIELDED) == 0)
			shld = -pl->shields;
		else
			shld = pl->shields;
		cplng(shld, txt);
		break;

	case 2:
	case 4:	/* pt is the hittee -- we don't hit stars, we hit planets */
		**txt = (char)(pt->builds << 3);
		if (pt->flags & EMPIRE)
			**txt |= (char)EPORT;
		else if (pt->flags & FEDDIE)
			**txt |= (char)FPORT;
		else
			**txt |= (char)NPORT;
		(*txt)++;
		*r = pt->rpos;
		*c = pt->cpos;
		cpsht(pt->rpos, txt);
		cpsht(pt->cpos, txt);
		if (what & RIP)	/* hittee died -- don't need more info */
			return;
		if (pt->builds == 5)
			cplng(pt->shields, txt);
		break;

	default:
		break;
	}
}

/*
 * ghitp -- store info about death of port or star in txt.
 * All we need is to know which it is, and location.
 * If port just attacked, need more info.
 */
void
ghitp(pt, what, txt, r, c)
PLANET *pt;
int what;
char **txt;
short *r, *c;
{
	**txt = '\0';
	if (what == PDIED || what == PATTK)
		**txt = (char)(pt->builds << 3);
	if (what == NDIED)
		**txt = (char)STAR;
	else if (pt->flags & EMPIRE)
		**txt |= (char)EPORT;
	else if (pt->flags & FEDDIE)
		**txt |= (char)FPORT;
	else
		**txt |= (char)NPORT;
	(*txt)++;
	cpsht(pt->rpos, txt);
	cpsht(pt->cpos, txt);
	*r = pt->rpos;
	*c = pt->cpos;
	/*** don't tell about shields (Tdocode doesn't look for them.
	if (what == PATTK && pt->builds == 5)
		cplng(pt->shields, txt);
	***/
}
