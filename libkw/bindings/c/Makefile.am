AM_CPPFLAGS = -I$(top_srcdir)/libesort

include_HEADERS = esortc.h
lib_LTLIBRARIES = libesortc.la

libesortc_la_SOURCES = esortc.cpp esortc.h
libesortc_la_LIBADD = $(top_builddir)/libesort/libesort.la
libesortc_la_LDFLAGS = -version-info @LTVERSION@
