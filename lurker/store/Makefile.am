dbdir = @dbdir@

INCLUDES = @STORAGE_CFLAGS@ -I${top_srcdir}/common

REQUESTS = message mindex search splash mbox attach thread jump

sbin_PROGRAMS = lurkerd
dist_db_DATA = DB_CONFIG
EXTRA_DIST = Design

lurkerd_SOURCES = \
	config.h   config.c   \
	decode.h   decode.c   \
	md5.h      md5.c      \
	flatfile.h flatfile.c \
	wbuffer.h  wbuffer.c  \
	indexer.h  indexer.c  \
	summary.h  summary.c  \
	mbox.h     mbox.c     \
	breader.h  breader.c  \
	search.h   search.c   \
	service.h  service.c  \
	main.c     btree.h    \
	expiry.c   expiry.h

lurkerd_LDADD = @STORAGE_LIBS@ ../common/libcommon.a ${LIBICONV}

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
