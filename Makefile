#
# Program: Aiwass
#

.c.obj:
	qcl -c  -W1 -Ze $*.c

aiwass.obj  : aiwass.c aiwass.h command.h

command.obj : command.c aiwass.h command.h lexicon.h qabalah.h numicon.h \
		\include\usr\webster.h

lexicon.obj : lexicon.c lexicon.h \include\usr\webster.h

numicon.obj : numicon.c lexicon.h qabalah.h numicon.h \
		\include\usr\webster.h 

qabalah.obj : qabalah.c qabalah.h \include\usr\webster.h

scriber.obj : scriber.c scriber.h

webster.obj : webster.c \include\usr\webster.h

aiwass.exe  : aiwass.obj command.obj lexicon.obj numicon.obj \
		qabalah.obj scriber.obj webster.obj
	link @aiwass.lnk /NOI /E $(LDFLAGS);
