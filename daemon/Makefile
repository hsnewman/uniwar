#
# DLIBSRC is daemon library
#
# DEBUG -- prints debugging msgs to stdout
#
#DEBUG = -D DEBUG
DEBUG =
CFLAGS = -g
#CFLAGS = -O
#CFLAGS =
DLIBSRC=\
	Dactivate.c \
	Dadjcoords.c \
	Dall_cmd.c \
	Dcleanup.c \
	Dcoordstr.c \
	Ddaemon.c \
	Ddamaged.c \
	Ddecnrgy.c \
	Ddirect.c \
	Ddlvrhit.c \
	Ddo_cmd.c \
	Ddo_pregame.c \
	Ddo_scan.c \
	Ddobapl.c \
	Ddobuild.c \
	Ddodamage.c \
	Ddodock.c \
	Ddolist.c \
	Ddomove.c \
	Ddophaser.c \
	Ddopoint.c \
	Ddoquit.c \
	Ddorepair.c \
	Ddoscan.c \
	Ddoset.c \
	Ddoshield.c \
	Ddostatus.c \
	Ddosumm.c \
	Ddotarg.c \
	Ddotell.c \
	Ddotorp.c \
	Ddousers.c \
	Denditall.c \
	Derror.c \
	Deveryturn.c \
	Dgetkey.c \
	Dgetmsgs.c \
	Dgetship.c \
	Dgettok.c \
	Dghit.c \
	Dhitdevs.c \
	Dhitpmsg.c \
	Dhotspot.c \
	Dinit.c \
	Dkillpl.c \
	Dlifegoeson.c \
	Dlistthem.c \
	Dloadcmd.c \
	Dneedscan.c \
	Dnewbaud.c \
	Dnewcmd.c \
	Dnova.c \
	Dphaser.c \
	Dplay.c \
	Dpmsg.c \
	Dpotshot.c \
	Drepair.c \
	Drnd.c \
	Dsaybyebye.c \
	Dscorefile.c \
	Dscoreit.c \
	Dsendcmd.c \
	Dtelldmg.c \
	Dtellthem.c \
	Dtorp.c \
	Duglobs.c \
	Duniverse.c \
	Dunlist.c

DLIBOBJS=\
	Dactivate.o \
	Dadjcoords.o \
	Dall_cmd.o \
	Dcleanup.o \
	Dcoordstr.o \
	Ddaemon.o \
	Ddamaged.o \
	Ddecnrgy.o \
	Ddirect.o \
	Ddlvrhit.o \
	Ddo_cmd.o \
	Ddomove.o \
	Ddophaser.o \
	Ddopoint.o \
	Ddo_pregame.o \
	Ddo_scan.o \
	Ddobapl.o \
	Ddobuild.o \
	Ddodamage.o \
	Ddodock.o \
	Ddolist.o \
	Ddoquit.o \
	Ddorepair.o \
	Ddoscan.o \
	Ddoset.o \
	Ddoshield.o \
	Ddostatus.o \
	Ddosumm.o \
	Ddotarg.o \
	Ddotell.o \
	Ddotorp.o \
	Ddousers.o \
	Denditall.o \
	Derror.o \
	Deveryturn.o \
	Dgetkey.o \
	Dgetmsgs.o \
	Dgetship.o \
	Dgettok.o \
	Dghit.o \
	Dhitdevs.o \
	Dhitpmsg.o \
	Dhotspot.o \
	Dinit.o \
	Dkillpl.o \
	Dlifegoeson.o \
	Dlistthem.o \
	Dloadcmd.o \
	Dneedscan.o \
	Dnewbaud.o \
	Dnewcmd.o \
	Dnova.o \
	Dphaser.o \
	Dplay.o \
	Dpmsg.o \
	Dpotshot.o \
	Drepair.o \
	Drnd.o \
	Dsaybyebye.o \
	Dscorefile.o \
	Dscoreit.o \
	Dsendcmd.o \
	Dtelldmg.o \
	Dtellthem.o \
	Dtorp.o \
	Duglobs.o \
	Duniverse.o \
	Dunlist.o

uniward:	$(DLIBOBJS)
	$(CC) $(DEBUG) $(CFLAGS) $(DLIBOBJS) -o uniward

all:	uniward
	@echo "all made"

#
# dlint is lint of daemon.
# "lint" currently stands for dlint.
#
dlint:
	lint $(DEBUG) $(DLIBSRC)

lint:	dlint

.c.o:
	$(CC) -c $(DEBUG) $(CFLAGS) $*.c
