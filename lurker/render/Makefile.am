wwwdir = @wwwdir@

INCLUDES = @RENDER_CFLAGS@ -I${top_srcdir}/common

www_PROGRAMS = @PACKAGE@.cgi @PACKAGE@.xslt @PACKAGE@-search.cgi
www_DATA = .htaccess

@PACKAGE@_cgi_SOURCES = \
	main.c \
	error.h

@PACKAGE@_xslt_SOURCES = \
	xslt.c

@PACKAGE@_search_cgi_SOURCES = \
	search.c \
	error.h

@PACKAGE@_xslt_LDADD = @RENDER_LIBS@
@PACKAGE@_search_cgi_LDADD = ../common/libcommon.a
