noinst_LIBRARIES = libcommon.a
noinst_PROGRAMS = test

libcommon_a_SOURCES = message.c stub.c message.h common.h io.h

test_SOURCES = main.c
test_LDADD = @STORAGE_LIBS@ libcommon.a
