fmtdir = @wwwdir@/fmt

fmt_DATA = \
	render.xsl render-html.xsl common.xsl \
	error.xsl splash.xsl message.xsl mindex.xsl search.xsl thread.xsl \
	english.xsl \
	basic.css

install-data-local:
	$(mkinstalldirs) ${fmtdir}
	ln -fs basic.css   ${fmtdir}/default.css
	ln -fs english.xsl ${fmtdir}/lang.xsl

uninstall-local:
	rm -f ${fmtdir}/default.css
	rm -f ${fmtdir}/lang.xsl
