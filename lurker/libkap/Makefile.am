noinst_LIBRARIES = libkap.a

bin_PROGRAMS = kapmake kapdump

libkap_a_SOURCES = \
	kap.h private.h \
	btree.c \
	kap.c

kapmake_SOURCES = kapmake.c
kapmake_LDADD = libkap.a

kapdump_SOURCES = kapdump.c
kapdump_LDADD = libkap.a
