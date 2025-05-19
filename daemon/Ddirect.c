#include "config.h"
#include "u.h"

#define MMSIZE	9	/* movemap size */

extern short **map;
extern int Mrows, Mcols;

/*
 * bitmap to speed movement if move is within MMSIZExMMSIZE
 * quadrant.  mmap[i][j] is bitmap of row and col increments
 * which, when followed, move an object in as straight a line
 * as possible between (0,0) and (i,j).  For example,
 * mmap[3][8] has rinc=0112 and cinc=0377; that is,
 *
 *	rincs = binary 01001010, cincs = binary 11111111
 *
 * Movement starts with the least significant bit, so an
 * object moving from (0,0) to (3,8) would follow the path
 *
 * (+ 1st bit of rincs, + 1st bit of cincs),
 * (+ 2nd bit of rincs, + 2nd bit of cincs), etc.  In the example:
 *
 * (+0,+1), (+1,+1), (+0,+1), (+1,+1), (+0,+1), (+0,+1), (+1,+1), (+0,+1)
 *
 * to arrive at (3,8)
 */
struct movemap {
	unsigned char rincs;
	unsigned char cincs;
} mmap[][MMSIZE] = {
    /* mmap[0][0-8] */
    {
	{'\0','\0'},
	{'\0','\1'}, {'\0','\3'}, {'\0','\7'}, {'\0','\17'},
	{'\0','\37'}, {'\0','\77'}, {'\0','\177'}, {'\0','\377'}, 
    },
    /* mmap[1][0-8] */
    {
	{'\1','\0'},
	{'\1','\1'}, {'\1','\3'}, {'\2','\7'}, {'\2','\17'},
	{'\4','\37'}, {'\4','\77'}, {'\10','\177'}, {'\10','\377'}, 
    },
    /* mmap[2][0-8] */
    {
	{'\3','\0'},
	{'\3','\1'}, {'\3','\3'}, {'\5','\7'}, {'\5','\17'},
	{'\12','\37'}, {'\22','\77'}, {'\42','\177'}, {'\42','\377'}, 
    },
    /* mmap[3][0-8] */
    {
	{'\7','\0'},
	{'\7','\2'}, {'\7','\5'}, {'\7','\7'}, {'\13','\17'},
	{'\25','\37'}, {'\25','\77'}, {'\52','\177'}, {'\112','\377'}, 
    },
    /* mmap[4][0-8] */
    {
	{'\17','\0'},
	{'\17','\2'}, {'\17','\5'}, {'\17','\13'}, {'\17','\17'},
	{'\33','\37'}, {'\55','\77'}, {'\125','\177'}, {'\252','\377'}, 
    },
    /* mmap[5][0-8] */
    {
	{'\37','\0'},
	{'\37','\4'}, {'\37','\12'}, {'\37','\25'}, {'\37','\33'},
	{'\37','\37'}, {'\67','\77'}, {'\135','\177'}, {'\255','\377'}, 
    },
    /* mmap[6][0-8] */
    {
	{'\77','\0'},
	{'\77','\4'}, {'\77','\22'}, {'\77','\25'}, {'\77','\55'},
	{'\77','\67'}, {'\77','\77'}, {'\167','\177'}, {'\273','\377'}, 
    },
    /* mmap[7][0-8] */
    {
	{'\177','\0'},
	{'\177','\10'}, {'\177','\42'}, {'\177','\52'}, {'\177','\125'},
	{'\177','\135'}, {'\177','\167'}, {'\177','\177'}, {'\357','\377'}, 
    },
    /* mmap[8][0-8] */
    {
	{'\377','\0'},
	{'\377','\10'}, {'\377','\42'}, {'\377','\112'}, {'\377','\252'},
	{'\377','\255'}, {'\377','\273'}, {'\377','\357'}, {'\377','\377'}, 
    },
};

/*
 * Return information about an object's trip as it
 * moves through space (without actually moving the
 * object.  (or,oc) is the original absolute coords
 * of the object.  (dr,dc) is the row and col distance
 * that object is to travel.
 *
 * If the object is obstructed by another object,
 * HITOBJ is returned.  If the object would leave
 * the galaxy, HITEDGE is returned.  In either of
 * these cases, the place at which the moving object
 * would be forced to stop is returned in (sr, sc).
 * In the case of HITOBJ, the obstructing object's
 * coords are returned in (br, bc).
 *
 * If the trip is uneventful, 0 is returned and the
 * object's final destination is returned in (br, bc);
 *
 * lev is the level of recursion.  maxmove cannot
 * be less than MAX(abs(dr), abs(dc)) if lev == 0, else
 * we return TOOFAR.
 *
 * We plan to move maxmove times.  [NOTE: for player moves,
 * this arg should be -1 and calling function should verify
 * that MAX(abs(dr), abs(dc)) < MRANGE.  For torps, this
 * arg should be TRANGE or TRANGE+1.]  If maxmove is greater than
 * MAX(abs(dr), abs(dc)), direct() is called recursively.
 * If maxmove is -1, it is ignored.
 *
 * Don't move further than maxmove, even if (dr,dc)
 * indicates that we should.  This is because direct()
 * may be called recursively to handle close range
 * torpedo shots that miss.
 *
 * If torpedoes are to be somewhat inaccurate at long distance,
 * this should be controlled by the calling routine.
 */
direct(or, oc, dr, dc, br, bc, sr, sc, maxmove, lev)
short or, oc, dr, dc;
register short *br, *bc;
short *sr, *sc;
short maxmove;
short lev;
{
	short ri, ci;			/* row and col increments */
	register unsigned char rm, cm;	/* row and col bitmaps */
	short count, i;
	struct movemap *mp;

	mp = &mmap[abs(dr)][abs(dc)];
	count = MAX(abs(dr), abs(dc));
	if (maxmove == -1)
		maxmove = count;
	if (lev == 0 && count > maxmove)
		return(TOOFAR);
	if (dr < 0)
		ri = -1;
	else
		ri = 1;
	if (dc < 0)
		ci = -1;
	else
		ci = 1;
	*br = or;
	*bc = oc;
	rm = mp->rincs;
	cm = mp->cincs;
	for (i = 0; i < count && i < maxmove; i++) {
		*sr = *br;
		*sc = *bc;
		if ((rm>>i)&1)
			*br += ri;
		if ((cm>>i)&1)
			*bc += ci;
		if (*br > Mrows || *br < 1 || *bc > Mcols || *bc < 1)
			return(HITEDGE);
		if (map[*br][*bc] != EMPTY)
			return(HITOBJ);
	}
	if (i < maxmove)
		return(direct(*br,*bc,dr,dc,br,bc,sr,sc,maxmove-i,lev+1));
	else
		return(0);
}

/*
 * incremental direct -- necessary when game is compiled to allow
 * players to move farther than MMSIZE spaces in any direction
 * (by raising MRANGE) or when torpedoes have a TRANGE greater
 * than MMSIZE.  May require floating point.  Not implemented
 * at present.
 *
 * For now, torpedoes are always on target, even at range TRANGE.
 * Later we may tell idirect() that we're dealing with a torp
 * so that it will introduce some randomness.
 */
/*
idirect()
{
	return(1);
}
*/
