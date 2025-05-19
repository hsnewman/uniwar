#include "config.h"
#include "u.h"

extern PLAYER ships[];
extern short usedship;
extern char Outbuf[];

void ghitr(), ghite(), ghitp(), cpsht(), cplng();

/*
 * hitpmsg()	-- tell people about a hit or event.
 * 'what' is what happened:
 *	FASER-- somebody hit by phaser.
 *	TORP  -- somebody hit by torpedo.
 *	NOVA  -- somebody hit by nova.	
 *	RIP|FASER-- somebody killed by phaser.
 *	RIP|TORP  -- somebody killed by torpedo.
 *	RIP|NOVA  -- somebody killed by nova.	
 *	PDIED -- port died (only used to report death to team members).
 *	NDIED -- star blew up (only used when nova happens due to chain
 *				reaction of novas).
 *	PATTK -- port attacked (report attack to team members).
 *	DISPLAC -- if victim was displaced.
 *
 * 'flag' indicates who did what.  Either pl1 hits pl2, pl1 hits pt2,
 * pt1 hits pl2, or pt1 hits pt2 depending whether flag is 1, 2, 3, or 4
 * respectively.
 * 'hit' is the size of the hit.
 *
 * Send the message to people in the vicinity.  When a base dies,
 * tell everybody on that side who's not in the vicinity.
 *
 * Messages are encoded for decoding by the terminal handler.  This
 * will ease the message traffic and simplify the daemon's job.
 *
 * Messages have headers of the form: mwwffffrrcc
 *	m is char message type (UmCODE).
 *	ww is short "what happened".
 *	ffff is long, player flags.
 *	rr is short row psn of player getting the msg.
 *	cc is short col psn of player getting the msg.
 *
 * The rest of the command depends on the ww field.
 * See comments within the code.
 */
hitpmsg(what, flag, hit, pl1, pl2, pt1, pt2)
int what, flag;
long hit;
PLAYER *pl1, *pl2;
PLANET *pt1, *pt2;
{
	short r, c;
	PLAYER *pl;
	int i;
	char *hdr, *txt, *savehdr;
	short msize;

	/* start the header */
	hdr = Outbuf+sizeof(short);
	txt = Outbuf+sizeof(short)+11;	/* after 11 byte header mwwffffrrcc */
	*hdr++ = UmCODE;
	cpsht((short)what, &hdr);

	/* construct rest of message */
	msize = -1;
	switch(what & WHATMASK) {
	case FASER:	/* somebody hit by phaser. */
	case TORP:	/* somebody hit by torpedo. */
	case NOVA:	/* somebody hit by nova. */
		ghitr(flag, pl1, pt1, hit, what, &txt);
		ghite(flag, pl2, pt2, what, &txt, &r, &c);
		break;

	case PDIED:	/* port died -- tell team members. */
	case NDIED:	/* star blew up due to chain reaction. */
	case PATTK:	/* port attacked (report attack to team members). */
		ghitp(pt1, what, &txt, &r, &c);
		break;

	default:
		break;
	}
	msize = txt - Outbuf - sizeof(short);

	/* for each player getting the message, the header is different */
	savehdr = hdr;
	for (i = usedship; i != -1; i = pl->list) {
		/*
		 * send the msg if:
		 * the player is in the vicinity OR
		 * (a base died AND the player has RADIO on)
		 */
		pl = &ships[i];

		/* don't talk to dead players and players in pregame */
		if ((pl->flags & DIED) || (pl->flags & ACTIVE) == 0)
			continue;

		/*
		 * If msg is not PDIED, send it to everyone in vicinity.
		 * Otherwise, send it to friendly ships with their
		 * radios on.
		 */
		if (((MAX(abs(pl->rpos - r), abs(pl->cpos - c)) < VICINITY) &&
			what != PDIED && what != PATTK) ||
			((what==PDIED || what==PATTK) && (pl->flags & RADIO) &&
			(pl->flags & ALIMASK) == (pt1->flags & ALIMASK))) {
			/* finish the header */
			cplng(pl->flags, &hdr);
			cpsht(pl->rpos, &hdr);
			cpsht(pl->cpos, &hdr);

			if (sendcmd(pl, msize) < 0)
				return(-1);
			hdr = savehdr;
		}
	}
	return(1);
}
