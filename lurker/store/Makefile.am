INCLUDES = @STORAGE_CFLAGS@ -I${top_srcdir}/common

sbin_PROGRAMS = @PACKAGE@d

@PACKAGE@d_SOURCES = \
	keyword.h keyword.c \
	config.c db.c \
	main.c globals.h records.h

@PACKAGE@d_LDFLAGS = @STORAGE_LIBS@
