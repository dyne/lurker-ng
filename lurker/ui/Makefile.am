fmtdir = ${wwwdir}/fmt
htmldir = ${wwwdir}

dist_fmt_DATA = \
	default.css common.js english.xsl common.xsl list.xsl message.xsl \
	mindex.xsl search.xsl splash.xsl thread.xsl

dist_html_DATA = index.html

install-data-local:
	for i in list message mindex search splash thread; do \
		$(mkinstalldirs) $(DESTDIR)$(htmldir)/$$i; \
		done