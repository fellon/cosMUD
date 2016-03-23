CC      = gcc
#PROF    = -p
NOCRYPT =
 
# Please read all the documents in the system dir and README.1st in main dir.
# By default, this MUD will start on port 4100 and IMC on port 4125.
 
# Alter the directory below and mud.h to set the debugger
#DMALLOC = -L/home/ucmm2/rm4/src/dmalloc/lib -ldmalloclp -ldmalloc -ldl
 
# Uncomment the two lines below if compiling on a Solaris box
#SOLARIS_FLAG = -Dsun -DSYSV
#SOLARIS_LINK = -lnsl -lsocket
 
# Uncomment the line below if you are getting undefined crypt errors
NEED_CRYPT = -lcrypt
 
DBUGFLG = -DREQUESTS

# Code support for Intermud-3. Comment out to disable.
I3 = -DI3 -DI3SMAUG
 
# Uncomment the line below if you want a performance increase though beware
# your core files may not be as much of a benefit if you do.
#OPT_FLAG = -finline-functions -funroll-loops -fdefer-pop -fstrength-reduce
 
C_FLAGS = $(OPT_FLAG) -g3 -O -Wall $(I3) $(PROF) $(NOCRYPT) $(DBUGFLG) -DSMAUG $(SOLARIS_FLAG)
L_FLAGS = $(OPT_FLAG) $(PROF) $(SOLARIS_LINK) $(NEED_CRYPT) $(DMALLOC)
 
O_FILES = act_comm.o act_info.o act_move.o act_obj.o act_wiz.o arena.o \
          axsmaug.o ban.o bank.o base64.o boards.o build.o clans.o comm.o \
          comments.o const.o db.o deity.o fight.o grub.o handler.o hashstr.o \
          hiscores.o hskelter.o i3.o ibuild.o ident.o imm_host.o immscore.o \
          interp.o locker.o magic.o makeobjs.o medit.o misc.o motd.o \
          mpxset.o mud_comm.o mud_prog.o oedit.o planes.o player.o \
          polymorph.o redit.o requests.o reset.o save.o services.o \
          shops.o skills.o special.o stat_obj.o suicide.o tables.o \
          track.o update.o websvr.o minilzo.o \
          ice.o icec.o icec-mercbase.o imc.o imc-config.o imc-events.o \
          imc-interp.o imc-mail.o imc-mercbase.o imc-util.o imc-version.o \
 
C_FILES = act_comm.c act_info.c act_move.c act_obj.c act_wiz.c arena.c \
          axsmaug.c ban.c bank.c base64.c boards.c build.c clans.c comm.c \
          comments.c const.c db.c deity.c fight.c grub.c handler.c hashstr.c \
          hiscores.c hskelter.c i3.c ibuild.c ident.c imm_host.c immscore.c \
          interp.c locker.c magic.c makeobjs.c medit.c misc.c motd.c \
          mpxset.c mud_comm.c mud_prog.c oedit.c planes.c player.c \
          polymorph.c redit.c requests.c reset.c save.c services.c \
          shops.c skills.c special.c stat_obj.c suicide.c tables.c \
          track.c update.c websvr.c minilzo.c \
          ice.c icec.c icec-mercbase.c imc.c imc-config.c imc-events.c \
          imc-interp.c imc-mail.c imc-mercbase.c imc-util.c imc-version.c \
 
H_FILES = bank.h bet.h mud.h olc.h ice.h icec.h icec-mercbase.h imc.h \
          imc-config.h imc-mercbase.h imc-mercdefs.h lzoconf.h minilzo.h \
 
all:
	make rmexe
 
rmexe: $(O_FILES)
	rm -f rmexe
	$(CC) $(L_FLAGS) -o rmexe $(O_FILES) $(DMALLOC)
	chmod g+w rmexe
	chmod a+x rmexe
	chmod g+w $(O_FILES)
 
.c.o: mud.h
	$(CC) -c $(C_FLAGS) $<
 
clean:
	rm -f *.o rmexe *~
