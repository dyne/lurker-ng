lib_LTLIBRARIES = libesort.la
include_HEADERS = \
	DbMan.h Failer.h File.h Master.h Memory.h Merger.h Source.h \
	Transaction.h View.h esort.h io.h

libesort_la_SOURCES = \
	Parameters.cpp Memory.cpp File.cpp Source.cpp Transaction.cpp \
	Merger.cpp Failer.cpp View.cpp Master.cpp DbMan.cpp
libesort_la_LDFLAGS = -version-info @LTVERSION@
