dbdir = @dbdir@

INCLUDES = @STORAGE_CFLAGS@ -I${top_srcdir}/common -I${top_srcdir}/libkap

REQUESTS = message mindex search splash mbox attach thread jump

sbin_PROGRAMS = lurkerd
db_DATA       = DB_CONFIG

EXTRA_DIST = Design DB_CONFIG

lurkerd_SOURCES = \
	config.h   config.c   \
	decode.h   decode.c   \
	md5.h      md5.c      \
	indexer.h  indexer.c  \
	summary.h  summary.c  \
	mbox.h     mbox.c     \
	search.h   search.c   \
	service.h  service.c  \
	expiry.c   expiry.h   \
	main.c     \
	url.c mailto.c quote.c art.c


lurkerd_LDADD = @STORAGE_LIBS@ ../common/libcommon.a ../libkap/libkap.a ${LIBICONV} @INTLLIBS@ -lm

install-data-local:
	@list='$(REQUESTS)'; for p in $$list; do \
	  echo "$(mkinstalldirs) $(DESTDIR)$(wwwdir)/$$p"; \
	  $(mkinstalldirs) $(DESTDIR)$(wwwdir)/$$p; \
	done
	-$(mkinstalldirs) $(DESTDIR)$(localstatedir)/run

uninstall-local:
	@list='$(REQUESTS)'; for p in $$list; do \
	  echo "rm -f $(DESTDIR)$(wwwdir)/$$p/*"; \
	  rm -f $(DESTDIR)$(wwwdir)/$$p/*; \
	  echo "rmdir $(DESTDIR)$(wwwdir)/$$p"; \
	  rmdir $(DESTDIR)$(wwwdir)/$$p; \
	done
	rm -f $(DESTDIR)$(localstatedir)/run/lurker.pid
	-rmdir $(DESTDIR)$(localstatedir)/run
