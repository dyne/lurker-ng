htmldir = ${wwwdir}

html_PROGRAMS = lurker.cgi jump.cgi keyword.cgi

AM_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/libesort
LDADD = ../common/libcommon.a ../libesort/libesort.a

lurker_cgi_SOURCES = \
	main.cpp art.cpp url.cpp mailto.cpp quote.cpp attach.cpp list.cpp \
	mbox.cpp message.cpp mindex.cpp search.cpp splash.cpp thread.cpp \
	Cache.cpp KeyReader.cpp Summary.cpp Threading.cpp \
	Cache.h KeyReader.h Search.h Summary.h Threading.h commands.h \
	error.h parse.h

jump_cgi_SOURCES = parse.cpp jump.cpp
keyword_cgi_SOURCES = parse.cpp keyword.cpp
