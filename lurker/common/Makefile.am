noinst_LIBRARIES = libcommon.a
noinst_PROGRAMS = test

libcommon_a_SOURCES = \
	common.h common.c \
	message.c message.h \
	stub.c io.h

test_SOURCES = main.c
test_LDADD = @STORAGE_LIBS@ libcommon.a
