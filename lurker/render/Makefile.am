cgidir = @cgi_bin_dir@

cgi_PROGRAMS = lurker.cgi jump.cgi keyword.cgi bounce.cgi

AM_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/libesort
LDADD = ../common/libcommon.a ../libesort/libesort.a

lurker_cgi_SOURCES = \
	main.cpp art.cpp url.cpp mailto.cpp quote.cpp attach.cpp list.cpp \
	mbox.cpp message.cpp mindex.cpp search.cpp splash.cpp thread.cpp \
	zap.cpp Cache.cpp Threading.cpp parse.cpp \
	Cache.h Threading.h commands.h error.h parse.h

jump_cgi_SOURCES = parse.cpp jump.cpp
keyword_cgi_SOURCES = parse.cpp keyword.cpp
bounce_cgi_SOURCES = parse.cpp bounce.cpp
