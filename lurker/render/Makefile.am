wwwdir = @wwwdir@

INCLUDES = @RENDER_CFLAGS@ -I${top_srcdir}/common

www_PROGRAMS = lurker.cgi lurker.xslt \
		lurker-search.cgi lurker-jump.cgi
www_DATA = .htaccess

lurker_cgi_SOURCES = \
	main.c \
	error.h

lurker_xslt_SOURCES = \
	xslt.c

lurker_search_cgi_SOURCES = \
	search.c \
	error.h

lurker_jump_cgi_SOURCES = \
	jump.c \
	error.h

lurker_xslt_LDADD = @RENDER_LIBS@ @INTLLIBS@
lurker_search_cgi_LDADD = ../common/libcommon.a @INTLLIBS@
lurker_jump_cgi_LDADD = @INTLLIBS@
