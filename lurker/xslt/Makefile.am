fmtdir = @wwwdir@/fmt

dist_fmt_DATA = \
	render.xsl render-html.xsl common.xsl \
	error.xsl splash.xsl message.xsl mindex.xsl search.xsl thread.xsl \
	english.xsl \
	basic.css

install-data-local:
	$(mkinstalldirs) $(DESTDIR)${fmtdir}
	ln -fs basic.css   $(DESTDIR)${fmtdir}/default.css
	ln -fs english.xsl $(DESTDIR)${fmtdir}/lang.xsl

uninstall-local:
	rm -f $(DESTDIR)${fmtdir}/default.css
	rm -f $(DESTDIR)${fmtdir}/lang.xsl
