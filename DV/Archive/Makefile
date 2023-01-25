# Generated automatically from Makefile.in by configure.
.IGNORE:

SHELL = /bin/sh

# PATHS
prefix = /usr/local
bindir = $(prefix)/bin
libdir = $(prefix)/lib
DV_HOME = /d/bh4/home/fransp/DV

# Programs
CTOF = mfi
PERL = perl
RPCGEN = rpcgen
RANLIB = ranlib
AR = ar
CC = gcc
F77 = f77

BBH_SYSTEM=LINUX

# Compiler Commands
F77_TRANSFORM = touch

LIBS       =   -lbbhutil  -lsv  -lforms  -lpthread  -lbbh_xyz_stream  -lutilio  -ljpeg  -lncarg  -lncarg_gks  -lncarg_c -lglut -lGLU -lGL -lXmu -lXi -lXext -lX11 -lm -lCliser -lg2c -lm
LDFLAGS    = -L/d2/usr/X11R6/lib -L/usr/local/lib  -L/usr/lib -L.

CC       = gcc
CFLAGS   = -c 
CPPFLAGS =  -DLINUX -D__ -DPORT_GROUP -DWant_c_files   -I/usr/include  -I/usr/X11R6/include -I$(DV_HOME)/ -I$(DV_HOME)/DV_gui/ -I$(DV_HOME)/vis/ -I$(DV_HOME)/Cliser/

CC_COMP  = $(CC) $(CFLAGS) $(CPPFLAGS)
CC_LOAD  = $(CC) $(LDFLAGS) -L$(DV_HOME)/Cliser/ 

F77      = f77
F77FLAGS = -g

F77_COMP   = $(F77) -c $(F77FLAGS)
F77_LOAD   = $(F77) $(F77FLAGS) $(LDFLAGS)

.f.o:
	$(F77_COMP) -c $*.f

.c.o:
	$(CC_COMP) -c $*.c

AUX = DV_gui util vis Cliser rnpl_diff/1d misc_f77 rnpl_diff/2d misc_f77 rnpl_diff/3d misc_f77

EXECUTABLES = DV sdftodv
LIBFILES = lib/default_opts.dvo

# Primary target

all: DV.o
	echo; echo "Making in $(AUX)"
	for f in $(AUX); do \
		(cd $$f; make) \
	done
	make DV

install: all
	if test ! -d $(prefix); then\
		mkdir $(prefix);\
	fi
	if test ! -d $(bindir); then\
		mkdir $(bindir);\
	fi
	if test ! -d $(libdir); then\
		mkdir $(libdir);\
	fi
	if test ! -d $(libdir)/DV; then\
		mkdir $(libdir)/DV;\
	fi
	cp DV util/sdftodv $(bindir)
	cp $(LIBFILES) $(libdir)/DV

DV.o: DV.c reg.h misc.h DV.h instr.h DV_gui/DV_forms.h
	$(CC_COMP) DV.c

reg.o: reg.c reg.h misc.h s_iter.h
	$(CC_COMP) -c reg.c

instr.o: instr.c instr.h reg.h s_iter.h
	$(CC_COMP) -c instr.c

misc.o: misc.c misc.h 
	$(CC_COMP) -c misc.c

fncs.o: fncs.c fncs.h reg.h misc.h s_iter.h
	$(CC_COMP) -c fncs.c

s_iter.o: s_iter.c s_iter.h reg.h 
	$(CC_COMP) -c s_iter.c

null_int.o: null_int.c fncs.h reg.h misc.h s_iter.h
	$(CC_COMP) -c null_int.c

OBJS	=	DV.o reg.o misc.o instr.o misc_f77/misc_f77.o fncs.o s_iter.o DV_gui/DV_forms.o DV_gui/DV_gui.o vis/send_bbh_xyz.o rnpl_diff/1d/updates_f1.o rnpl_diff/2d/updates_f2.o rnpl_diff/3d/updates_f3.o vis/send_accept_sdf.o vis/send_gl_canvas.o misc_f77/2Dclusterer.o misc_f77/dc_clusterer.o vis/scr_save.o vis/ncar_out.o null_int.o

DV: $(OBJS)
	$(CC_LOAD) $(OBJS) $(LIBS) $(LIBGL) -o DV
	cp DV bin

clean:
	echo; echo "Cleaning up"
	for f in $(AUX); do \
		(cd $$f; make clean) \
	done
	(cd bin; rm $(EXECUTABLES) > /dev/null 2>&1) 
	rm $(EXECUTABLES) > /dev/null 2>&1
	rm *.o > /dev/null 2>&1
	rm *.sdf > /dev/null 2>&1
	rm config.cache config.log config.status > /dev/null 2>&1
	rm Makefile > /dev/null 2>&1
