noinst_LIBRARIES = libkap.a

bin_PROGRAMS = kapmake kapdump

libkap_a_SOURCES = \
	kap.h private.h avl.h \
	btree.c append.c wbuffer.c kap.c

kapmake_SOURCES = kapmake.c
kapmake_LDADD = libkap.a -lm @MAINT@ -ldmalloc

kapdump_SOURCES = kapdump.c
kapdump_LDADD = libkap.a -lm @MAINT@ -ldmalloc

tests:	test-append test-btree test-full test-avl profile
	./test-avl
	./test-btree
	./test-append
	cat *.c *.cc *.h | ./test-full

test-append: test-append.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@ -lm

test-avl: test-avl.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@ -lm

test-btree: test-btree.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@ -lm

test-full: test-full.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@ -lm

profile: profile.cc libkap.a
	g++ -Wall -O2 -g $< libkap.a -o $@ -lm

clean-local:
	rm -f test-append test-btree test-full test-avl profile
	rm -f *.btree *.append
