imgsdir = @wwwdir@/imgs

IMGS =	a.png b.png c.png d.png e.png \
	f.png g.png h.png i.png j.png k.png \
	next.png prev.png first.png last.png

imgs_DATA = ${IMGS}
EXTRA_DIST = ${IMGS}

install-data-local:
	$(mkinstalldirs) $(DESTDIR)${imgsdir}
	ln -fs ../imgs   $(DESTDIR)${wwwdir}/thread/i

uninstall-local:
	rm -f $(DESTDIR)${wwwdir}/thread/i
