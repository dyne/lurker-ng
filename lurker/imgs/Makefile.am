imgsdir = @wwwdir@/imgs

dist_imgs_DATA = a.png b.png c.png d.png e.png f.png g.png h.png

install-data-local:
	$(mkinstalldirs) ${imgsdir}
	ln -fs ../imgs   ${wwwdir}/thread/i

uninstall-local:
	rm -f ${wwwdir}/thread/i
