EXTRA_DIST = lurker-drop-rlimit.cpp lurker-regenerate

bin_PROGRAMS = lurker-index lurker-list lurker-params

AM_CPPFLAGS = -I$(top_srcdir)/common -I$(top_srcdir)/libesort
LDADD = ../common/libcommon.a ../libesort/libesort.a

lurker_index_SOURCES = Index.cpp main.cpp getdate.cpp Index.h getdate.h
lurker_list_SOURCES = list.cpp
lurker_params_SOURCES = params.cpp

dist_man_MANS = lurker-index.1 lurker-list.1 lurker-params.1
