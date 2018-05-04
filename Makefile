# Generated automatically from Makefile.in by configure.
# Seriously hacked by K. Richard Pixley <rich@cygnus.com>.

srcdir = .
prefix = c:/progra~1/byacc
exec_prefix = ${prefix}

bindir = ${exec_prefix}/bin
libdir = ${exec_prefix}/lib

datadir = ${prefix}/share

mandir = ${prefix}/man
man1dir = $(mandir)/man1
man2dir = $(mandir)/man2
man3dir = $(mandir)/man3
man4dir = $(mandir)/man4
man5dir = $(mandir)/man5
man6dir = $(mandir)/man6
man7dir = $(mandir)/man7
man8dir = $(mandir)/man8
man9dir = $(mandir)/man9
infodir = ${prefix}/info
includedir = ${prefix}/include


INSTALL = /usr/bin/install -cpD
INSTALL_PROGRAM = /usr/bin/install -cpsD
INSTALL_DATA = /bin/install -cp
EXEEXT = 
TEMP=mkstemp.o
#TEMP=

AR = ar
AR_FLAGS = qv
MAKEINFO = makeinfo

HDRS	      = defs.h

CFLAGS	      = -g -DNORAND

MORE_CFLAGS   = -DNDEBUG  -DRETSIGTYPE=void

LDFLAGS	      =

LIBS	      =

LINKER	      = $(CC)

MAKEFILE      = Makefile

OBJS	      = closure.o \
		error.o \
		hardcode.o \
		lalr.o \
		lex.o \
		ll1.o \
		lr0.o \
		main.o \
		mkpar.o \
		output.o \
		reader.o \
		skeleton.o \
		symtab.o \
		verbose.o \
		warshall.o $(TEMP)

PRINT	      = pr -f -l88

PROGRAM	      = byaccll$(EXEEXT)

SRCS	      = closure.c \
		error.c \
		lalr.c \
		lr0.c \
		main.c \
		mkpar.c \
		output.c \
		reader.c \
		skeleton.c \
		symtab.c \
		verbose.c \
		warshall.c

STAGESTUFF = $(OBJS) $(PROGRAM)

.c.o:
	$(CC) -c $(CFLAGS) $(MORE_CFLAGS) $<

all:		$(PROGRAM)
.PHONY: check installcheck
check:
installcheck:

info: byacc.info

dvi:

install-info:	byacc.info
	$(INSTALL) byacc.info $(infodir)

clean-info:;	rm -f byacc.info*

$(PROGRAM):     $(OBJS) $(LIBS)
		$(LINKER) $(CFLAGS) $(LDFLAGS) -o $(PROGRAM) $(OBJS) $(LIBS) $(LOADLIBES)

byacc.info:	byacc.texi
		$(MAKEINFO) -o byacc.info $(srcdir)/byacc.texi

clean:		clean-info
		rm -f $(OBJS) $(PROGRAM)

mostlyclean:	clean

distclean:	clean
		rm -f Makefile config.cache config.log config.status 

maintainer-clean realclean:	distclean

clobber:;	@rm -f $(OBJS) $(PROGRAM)

depend:;	@mkmf -f $(MAKEFILE) PROGRAM=$(PROGRAM) DEST=$(DEST)

index:;		@ctags -wx $(HDRS) $(SRCS)

install:	all
		$(INSTALL_PROGRAM) $(PROGRAM) $(bindir)/$(PROGRAM)
		$(INSTALL_DATA) $(srcdir)/manpage $(man1dir)/byacc.1

listing:;	@$(PRINT) Makefile $(HDRS) $(SRCS) | lpr

lint:;		@lint $(SRCS)

program:        $(PROGRAM)

tags:           $(HDRS) $(SRCS); @ctags $(HDRS) $(SRCS)

etags: 		TAGS

TAGS: force
	etags `$(MAKE) ls`

ls:
	for i in $(HDRS) $(SRCS) ; do \
		echo $(srcdir)/$$i ; \
	done

force:

Makefile: Makefile.in # config.status
	$(SHELL) config.status

config.status: configure
	$(SHELL) config.status --recheck

###
closure.o: defs.h
error.o: defs.h
lalr.o: defs.h
lr0.o: defs.h
main.o: defs.h
mkpar.o: defs.h
output.o: defs.h
reader.o: defs.h
skeleton.o: defs.h
symtab.o: defs.h
verbose.o: defs.h
warshall.o: defs.h
