noinst_LIBRARIES = libcommon.a
noinst_PROGRAMS = test
EXTRA_DIST = system.h.in

libcommon_a_SOURCES = \
	common.h common.c \
	message.c message.h \
	stub.c io.h protocol.h prefix.h

test_SOURCES = main.c
test_LDADD = @STORAGE_LIBS@ libcommon.a
