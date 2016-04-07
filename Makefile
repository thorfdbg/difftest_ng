#######################################################################
##
## $Id: Makefile,v 1.19 2016/04/07 10:44:49 thor Exp $
##
#######################################################################
## Makefile for the difftest_ng project,
## THOR Software, 2. Jul. 2003, Thomas Richter for
## Accusoft
#######################################################################
##
## 
.PHONY:		clean debug final valgrind valfinal coverage all install doc dox distrib \
		verbose profile profgen profuse Distrib.zip view realclean \
		uninstall reconfigure link linkglobal linkprofuse linkprofgen linkprof
		help

all:		debug

help:
		@ echo "difftest_ng Linux Edition. Available make targets:"
		@ echo "clean     : cleanup directories, remove object code"
		@ echo "            a second clean will remove autoconf generated files"
		@ echo "debug     : debug-build target without optimizations and "
		@ echo "            debugger support (default)"
		@ echo "final     : final build with optimizer enabled and no debugging"
		@ echo "valgrind  : debug build without internal memory munger, allows"
		@ echo "            to detect memory leaks with valgrind"
		@ echo "valfinal  : final build for valgrind with debug symbols"
		@ echo "coverage  : build for coverage check tools"
		@ echo "doc       : build the documentation"
		@ echo "distrib   : build distribution zip archive"
		@ echo "verbose   : debug build with verbose logging"
		@ echo "profile   : build with profiler support"
		@ echo "profgen   : ICC build target for collecting profiling information"
		@ echo "profuse   : second stage optimizer, uses profiling information"
		@ echo "            collected with 'make profgen' generated target"
		@ echo "install   : install j2k into ~/bin/wavelet"
		@ echo "uninstall : remove j2k from ~/bin/wavelet"

#####################################################################
## Varous Autoconf related settings                                ##
#####################################################################

configure:	configure.in autoconfig.h.in
	@ -$(AUTOCONF)

autoconfig.h.in:	configure.in
	@ -$(AUTOHEADER)

autoconfig.h:	autoconfig.h.in configure.in configure
	@ if test ! -f autoconfig.h; then ./configure CXX=$(COMPILER) CC=$(COMPILER); fi
	@ touch autoconfig.h

##
## faked rule for the automake file to shut up
## realclean and clean targets. All others depend
## on autoconfig.h and thus on this file anyhow.
automakefile:	
	@ echo "" >automakefile

##
##
## The following variable defines the compiler we use
## for running the code. This is the default version
## Use 
## $ make COMPILER=icc
## on the command line to test the same stuff with a 
## different compiler.
##
##COMPILER	=	gcc
##
## Include autoconf settings as soon as we have them
## will be included by recursive make process
##
-include	automakefile

#####################################################################
#####################################################################
##
## Build up the absolute path for the makefile containing the
## compiler specific definitions
##
ifdef		COMPILER
##
## User selection
COMPILER_SETTINGS=	$(shell pwd)/Makefile_Settings.$(COMPILER)
else
ifdef		SETTINGS
##
## Autoconf selection
COMPILER_SETTINGS=	$(shell pwd)/Makefile_Settings.$(SETTINGS)
else
##
## Fallback for realclean, clean
COMPILER_SETTINGS=	$(shell pwd)/Makefile_Settings.gcc
endif
endif
AUTOMAKEFILE=	$(shell pwd)/automakefile

##
## Include the project specific definitions

include		Makefile.template

#####################################################################
#####################################################################

ifdef	PREFIX
INSTALLDIR	= $(PREFIX)
else
INSTALLDIR	= $(HOME)/wavelet/bin
endif

##
##

##
## sub-directory libraries, automatically computed
##

DIRLIBS		=	$(foreach dir,$(DIRS),$(dir).dir)
BUILDLIBS	=	$(foreach dir,$(DIRS),$(dir).build)
FINALLIBS	=	$(foreach dir,$(DIRS),$(dir)/lib$(dir).a)
SHAREDLIBS	=	$(foreach dir,$(DIRS),$(dir)/lib_$(dir).a)
OBJECTLIST	=	$(foreach dir,$(DIRS),$(dir)/objects.list)
LIBOBJECTLIST	=	$(foreach dir,$(DIRS),$(dir)/libobjects.list)

%.dir:
		@ $(MAKE) COMPILER_SETTINGS=$(COMPILER_SETTINGS) AUTOMAKEFILE=$(AUTOMAKEFILE) \
		--no-print-directory -C $* sub$(TARGET)

%.build:
		@ $(MAKE) COMPILER_SETTINGS=$(COMPILER_SETTINGS) AUTOMAKEFILE=$(AUTOMAKEFILE) \
		MAKEFILE=$(shell if test -f $*/Makefile.$(HWTYPE); then echo Makefile.$(HWTYPE); else echo Makefile;fi) \
		--no-print-directory -C $* -f \
		$(shell if test -f $*/Makefile.$(HWTYPE); then echo Makefile.$(HWTYPE); else echo Makefile;fi) \
		sub$(TARGET)

echo_settings:
		@ $(ECHO) "Using $(CXX) $(OPTIMIZER) $(CFLAGS) $(GSL_CFLAGS) $(PNG_CFLAGS) $(PTHREADCFLAGS) $(EXR_CFLAGS)"

#####################################################################
#####################################################################
##
## Various link targets
##


linkflex:
		@ $(ECHO) "Linking..."
		@ $(CAT) $(OBJECTLIST) >objects.list
		@ sed -e 's/cmd\/main.o/cmd\/$(MAIN).o/' <objects.list >objects.list.tmp
		@ mv objects.list.tmp objects.list
		@ $(LD) $(LDFLAGS) $(PTHREADLDFLAGS) `cat objects.list` \
		  $(LDLIBS) $(PTHREADLIBS) $(GSL_LDFLAGS) $(PNG_LDFLAGS) $(EXR_LDFLAGS) -o $(MAIN)

link:
		@ $(ECHO) "Linking..."
		@ $(CAT) $(OBJECTLIST) >objects.list
		@ $(LD) $(LDFLAGS) $(PTHREADLDFLAGS) `cat objects.list` \
		  $(LDLIBS) $(PTHREADLIBS) $(GSL_LDFLAGS) $(PNG_LDFLAGS) $(EXR_LDFLAGS) -o difftest_ng

linkstatic:
		@ $(ECHO) "Linking..."
		@ $(CAT) $(OBJECTLIST) >objects.list
		@ $(LD) $(LDFLAGS) `cat objects.list` \
		  $(LDLIBS) $(PTHREADLIBS) $(GSL_LDFLAGS) $(PNG_LDFLAGS) $(EXR_LDFLAGS) -s -static -o difftest_ng
		@ strip difftest_ng

linkglobal:
		@ $(ECHO) "Linking..."
		@ $(CAT) $(OBJECTLIST) >objects.list
		@ $(LD) $(LDFLAGS) $(PTHREADLDFLAGS) $(GLOBFLAGS) `cat objects.list` \
		  $(LDLIBS) $(PTHREADLIBS) $(GSL_LDFLAGS) $(PNG_LDFLAGS) $(EXR_LDFLAGS) -o difftest_ng

linkprofgen:
		@ $(ECHO) "Linking..."
		@ $(CAT) $(OBJECTLIST) >objects.list
		@ $(LD) $(LDFLAGS) $(PTHREADLDFLAGS) $(PROFGEN) `cat objects.list` \
		  $(LDLIBS) $(PTHREADLIBS) $(GSL_LDFLAGS) $(PNG_LDFLAGS) $(EXR_LDFLAGS) -o difftest_ng
linkprofuse:
		@ $(ECHO) "Linking..."
		@ $(CAT) $(OBJECTLIST) >objects.list
		@ $(LD) $(LDFLAGS) $(PTHREADLDFLAGS) $(GLOBFLAGS) $(PROFUSE) `cat objects.list` \
		  $(LDLIBS) $(PTHREADLIBS) $(GSL_LDFLAGS) $(PNG_LDFLAGS) $(EXR_LDFLAGS) -o difftest_ng

linkprof:
		@ $(ECHO) "Linking..."
		@ $(CAT) $(OBJECTLIST) >objects.list
		@ $(LD) $(LDFLAGS) $(PTHREADLDFLAGS) $(LDPROF) `cat objects.list` \
		  $(LDLIBS) $(PTHREADLIBS) $(GSL_LDFLAGS) $(PNG_LDFLAGS) $(EXR_LDFLAGS)-o difftest_ng

linkcoverage:
		@ $(ECHO) "Linking..."
		@ $(CAT) $(OBJECTLIST) >objects.list
		@ $(LD) $(LDFLAGS) $(PTHREADLDFLAGS) $(LDCOVERAGE) `cat objects.list` \
		  $(LDLIBS) $(PTHREADLIBS) $(GSL_LDFLAGS) $(PNG_LDFLAGS) $(EXR_LDFLAGS) -o difftest_ng

#####################################################################
#####################################################################
##
## Various make targets
##

stripe	:	autoconfig.h	
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="debug"
	@ $(MAKE) --no-print-directory -C cmd -f Makefile stripe.o
	@ $(MAKE) --no-print-directory linkflex MAIN="stripe"

debug	:	autoconfig.h	
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory link

verbose	:	autoconfig.h	
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory link

final	:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory link

static	:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory linkstatic

global	:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory linkglobal

profgen	:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory linkprofgen

profuse	:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory linkprofuse

profile	:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory linkprof

valgrind:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory link

valfinal:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory link

coverage:	autoconfig.h
	@ $(MAKE) --no-print-directory echo_settings $(BUILDLIBS) \
	TARGET="$@"
	@ $(MAKE) --no-print-directory linkcoverage

clean	:
	@ find . -name "*.d" -exec rm {} \;
	@ $(MAKE) --no-print-directory $(BUILDLIBS) \
	TARGET="$@"
	@ rm -rf *.dpi *.so difftest_ng gmon.out core Distrib.zip objects.list libobjects.list libj2k.so
	@ if test -f "doc/Makefile"; then $(MAKE) --no-print-directory -C doc clean; fi
	@ rm -rf dox/html

reconfigure	:	clean
	@ rm -rf autoconfig.h automakefile
	@ $(MAKE) COMPILER=$(COMPILER) autoconfig.h

distclean	:	realclean
	@ $(MAKE) configure
	@ $(MAKE) autoconfig.h.in

realclean	:	clean
	@ rm -f *.j2k *.ppm *.pgm *.bmp *.pgx *.pgx_?.h *.pgx_?.raw *.jp2 *.jpc cachegrind.out* gmon.out *.zip
	@ find . -name "*.da"   -exec rm {} \;
	@ find . -name "*.bb"   -exec rm {} \;
	@ find . -name "*.bbg"  -exec rm {} \;
	@ find . -name "*.gcov" -exec rm {} \;
	@ find . -name "*.dyn"  -exec rm {} \;
	@ find . -name "*.dpi"  -exec rm {} \;
	@ rm -rf /tmp/*.dyn /tmp/*.dpi
	@ rm -rf automakefile autoconfig.h configure

distrib	:	Distrib.zip

Distrib.zip	:	doc dox configure autoconfig.h.in
	@ touch configure.in
	@ sleep 2
	@ touch autoconfig.h.in
	@ sleep 2
	@ touch configure
	@ $(MAKE) --no-print-directory $(DIRLIBS) TARGET="zip"
	@ $(ZIPASCII) -r Distrib.zip README README.License config.h
	@ $(ZIPASCII) -r Distrib.zip vs10.0/difftest_ng/difftest_ng.sln vs10.0/difftest_ng/difftest_ng/difftest_ng.vcxproj
	@ $(ZIPASCII) -r Distrib.zip vs9.0/difftest_ng/difftest_ng.sln vs9.0/difftest_ng/difftest_ng/difftest_ng.vcproj
	@ $(ZIP) -r Distrib.zip Makefile Makefile.template Makefile_Settings.*
	@ $(ZIP) -r Distrib.zip configure configure.in automakefile.in autoconfig.h.in

install		:
	@ mkdir -p $(INSTALLDIR)
	@ cp difftest_ng $(INSTALLDIR)
	@ cp difftest_ng $(INSTALLDIR)/difftest_ng

uninstall	:
	@ rm -rf $(INSTALLDIR)/difftest_ng
