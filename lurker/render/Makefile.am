wwwdir = @wwwdir@

INCLUDES = @RENDER_CFLAGS@ -I${top_srcdir}/common

www_PROGRAMS = @PACKAGE@.cgi
dist_www_DATA = .htaccess

@PACKAGE@_cgi_SOURCES = \
	handler.h \
	search.c \
	thread.c \
	message.c \
	mindex.c \
	tindex.c \
	main.c

@PACKAGE@_cgi_LDADD = @RENDER_LIBS@ ../common/libcommon.a
