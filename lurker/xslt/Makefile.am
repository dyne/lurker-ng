fmtdir = @wwwdir@/fmt

fmtdir_DATA = \
	render.xsl render-html.xsl common.xsl \
	error.xsl \
	english.xsl \
	basic.css

install-data-local:
	$(mkinstalldirs) ${fmtdir}
	ln -s basic.css   ${fmtdir}/default.css
	ln -s english.xsl ${fmtdir}/lang.xsl

uninstall-local:
	rm -f ${fmtdir}/default.css
	rm -f ${fmtdir}/lang.xsl
