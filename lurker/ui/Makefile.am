htmldir = @default_www_dir@
uidir = $(htmldir)/ui

dist_html_DATA = index.html
dist_ui_DATA = \
	default.css common.js lang.xsl common.xsl list.xsl message.xsl \
	mindex.xsl search.xsl splash.xsl thread.xsl lang.xml \
	ca.xml da.xml de.xml el.xml en.xml es.xml fi.xml fr.xml gl.xml \
	hu.xml it.xml ja.xml nl.xml pl.xml pt-BR.xml pt.xml
	
# Create the directories for dynamic content
install-data-local:
	for i in attach list mbox message mindex search splash thread zap; do \
		$(mkinstalldirs) $(DESTDIR)$(htmldir)/$$i; \
		done
	touch $(DESTDIR)$(htmldir)/lurker.docroot

uninstall-local:
	rm -f $(DESTDIR)$(htmldir)/lurker.docroot
