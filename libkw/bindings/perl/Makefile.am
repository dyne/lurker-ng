AM_CPPFLAGS = @PERL_INC@ -I$(top_srcdir)/libesort

lib_LTLIBRARIES = KW.la
EXTRA_DIST = KW.xs KW.pm

nodist_KW_la_SOURCES = KW.cpp
KW_la_LIBADD = @PERL_LIB@ $(top_builddir)/libesort/libesort.la
KW_la_LDFLAGS = -module

KW.cpp:	KW.xs
	@XSUBPP@ $< > $@.tmp && mv $@.tmp $@
