EXTRA_DIST = lurker-drop-rlimit.cpp lurker-regenerate \
             lurker-index.sgml lurker-list.sgml \
             lurker-params.sgml lurker-search.sgml \
             lurker-regenerate.sgml

bin_PROGRAMS = lurker-index lurker-list lurker-params lurker-search

AM_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/libesort
LDADD = ../common/libcommon.a ../libesort/libesort.a

lurker_index_SOURCES = Index.cpp main.cpp getdate.cpp Index.h getdate.h
lurker_list_SOURCES = list.cpp
lurker_params_SOURCES = params.cpp
lurker_search_SOURCES = search.cpp

dist_man_MANS = lurker-index.1 lurker-list.1 lurker-params.1 \
		lurker-search.1 lurker-regenerate.1

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(vardir)

if MAINTAINER_MODE
lurker-index.1: lurker-index.sgml
	docbook-to-man $< > $@
lurker-list.1: lurker-list.sgml
	docbook-to-man $< > $@
lurker-params.1: lurker-params.sgml
	docbook-to-man $< > $@
lurker-search.1: lurker-search.sgml
	docbook-to-man $< > $@
lurker-regenerate.1: lurker-regenerate.sgml
	docbook-to-man $< > $@
endif
