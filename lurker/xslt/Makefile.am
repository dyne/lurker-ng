wwwdir = @wwwdir@

EXTRA_DIST = message.xslt mindex.xslt search.xslt

install-data-local:
	@list='$(EXTRA_DIST)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|.xslt$$||'`"; \
	  echo "$(mkinstalldirs) $(DESTDIR)$(wwwdir)/$$f"; \
	  $(mkinstalldirs) $(DESTDIR)$(wwwdir)/$$f; \
	  echo "$(INSTALL_DATA) $$d$$p $(DESTDIR)$(wwwdir)/$$f/render.xslt"; \
	  $(INSTALL_DATA) $$d$$p $(DESTDIR)$(wwwdir)/$$f/render.xslt; \
	done

uninstall-local:
	@list='$(EXTRA_DIST)'; for p in $$list; do \
	  if test -f "$$p"; then d=; else d="$(srcdir)/"; fi; \
	  f="`echo $$p | sed -e 's|.xslt$$||'`"; \
	  echo "rm -f $(DESTDIR)$(wwwdir)/$$f/render.xslt"; \
	  rm -f $(DESTDIR)$(wwwdir)/$$f/render.xslt; \
	done
	