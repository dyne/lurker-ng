fmtdir = @wwwdir@/fmt

XSL =	render.xsl render-html.xsl common.xsl error.xsl redirect.xsl \
	splash.xsl message.xsl mindex.xsl search.xsl thread.xsl \
	english.xsl deutsch.xsl \
	basic.css

INDEX =	index.html index.xsl

fmt_DATA = ${XSL}
www_DATA = ${INDEX}

EXTRA_DIST = ${XSL} ${INDEX}

install-data-local:
	$(mkinstalldirs) $(DESTDIR)${fmtdir}
	ln -fs basic.css   $(DESTDIR)${fmtdir}/default.css
	ln -fs english.xsl $(DESTDIR)${fmtdir}/lang.xsl

uninstall-local:
	rm -f $(DESTDIR)${fmtdir}/default.css
	rm -f $(DESTDIR)${fmtdir}/lang.xsl
