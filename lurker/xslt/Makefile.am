wwwdir = @wwwdir@

XSLTS = message mindex

install-data-hook:
	@list='$(XSLTS)'; for p in $$list; do \
	  if test -f "$$p.xslt"; then d=; else d="$(srcdir)/"; fi; \
	  echo "$(mkinstalldirs) $(DESTDIR)$(wwwdir)/$$p"; \
	  $(mkinstalldirs) $(DESTDIR)$(wwwdir)/$$p; \
	  echo "$(INSTALL_DATA) $$d$$p.xslt $(DESTDIR)$(wwwdir)/$$p/render.xslt"; \
	  $(INSTALL_DATA) $$d$$p.xslt $(DESTDIR)$(wwwdir)/$$p/render.xslt; \
	done
