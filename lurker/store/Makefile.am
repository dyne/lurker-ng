dbdir = @dbdir@

INCLUDES = @STORAGE_CFLAGS@ -I${top_srcdir}/common

sbin_PROGRAMS = @PACKAGE@d
dist_db_DATA = DB_CONFIG
EXTRA_DIST = Design

@PACKAGE@d_SOURCES = \
	config.h   config.c   \
	md5.h      md5.c      \
	flatfile.h flatfile.c \
	wbuffer.h  wbuffer.c  \
	indexer.h  indexer.c  \
	summary.h  summary.c  \
	mbox.h     mbox.c     \
	breader.h  breader.c  \
	search.h   search.c   \
	service.h  service.c  \
	main.c     btree.h    \
	expiry.c   expiry.h

@PACKAGE@d_LDADD = @STORAGE_LIBS@ ../common/libcommon.a
