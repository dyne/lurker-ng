AM_CPPFLAGS = @PERL_INC@ -I$(top_srcdir)/libesort

perldir = $(libdir)/perl/5.8.0
autodir = $(perldir)/auto/KW

perl_DATA = KW.pm
auto_LTLIBRARIES = KW.la
EXTRA_DIST = KW.xs KW.pm
CLEANFILES = KW.cpp perlxsi.c

nodist_KW_la_SOURCES = KW.cpp
KW_la_LIBADD = @PERL_LIB@ $(top_builddir)/libesort/libesort.la
KW_la_LDFLAGS = -module -avoid-version

KW.cpp:	KW.xs
	@XSUBPP@ $< > $@.tmp && mv $@.tmp $@
