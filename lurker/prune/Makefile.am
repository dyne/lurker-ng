EXTRA_DIST = lurker-prune.sgml
bin_PROGRAMS = lurker-prune
dist_man_MANS = lurker-prune.1

AM_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/libesort
LDADD = ../common/libcommon.a ../libesort/libesort.a

lurker_prune_SOURCES = \
	prune.cpp PTable.cpp attach.cpp list.cpp mbox.cpp message.cpp \
	mindex.cpp search.cpp splash.cpp thread.cpp PTable.h

if MAINTAINER_MODE
lurker-prune.1: lurker-prune.sgml     
	docbook-to-man $< > $@
endif
