noinst_LIBRARIES = libkap.a

bin_PROGRAMS = kapmake kapdump

libkap_a_SOURCES = \
	kap.h private.h avl.h \
	btree.c append.c \
	kap.c

kapmake_SOURCES = kapmake.c
kapmake_LDADD = libkap.a

kapdump_SOURCES = kapdump.c
kapdump_LDADD = libkap.a

tests:	test-append test-btree test-full profile

test-append: test-append.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@

test-btree: test-btree.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@

test-full: test-full.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@

profile: profile.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@

clean-local:
	rm -f test-append test-btree test-full profile
	rm -f *.btree *.append
