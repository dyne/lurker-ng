AM_CPPFLAGS = -I@PYTHON_PREFIX@/include/python@PYTHON_VERSION@ -I$(top_srcdir)/libesort

pythondir = @PYTHON_EXEC_PREFIX@/lib/python@PYTHON_VERSION@/site-packages

python_LTLIBRARIES = ESortmodule.la

ESortmodule_la_SOURCES = ESortmodule.cpp
ESortmodule_la_LIBADD = -L@PYTHON_EXEC_PREFIX@/lib -lpython@PYTHON_VERSION@ $(top_builddir)/libesort/libesort.la
ESortmodule_la_LDFLAGS = -module -avoid-version
