AM_CPPFLAGS = @PERL_INC@ -I$(top_srcdir)/libesort

perldir = $(libdir)/perl/5.8.0
autodir = $(perldir)/auto/ESort

perl_DATA = ESort.pm
auto_LTLIBRARIES = ESort.la
EXTRA_DIST = ESort.xs ESort.pm ESort.map
CLEANFILES = ESort.cpp perlxsi.c

nodist_ESort_la_SOURCES = ESort.cpp
ESort_la_LIBADD = @PERL_LIB@ $(top_builddir)/libesort/libesort.la
ESort_la_LDFLAGS = -module -avoid-version

ESort.cpp:	ESort.xs ESort.map
	@XSUBPP@ -typemap ESort.map $< > $@.tmp && mv $@.tmp $@
