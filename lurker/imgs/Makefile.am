imgsdir = @wwwdir@/imgs

dist_imgs_DATA = \
	a.png b.png c.png d.png e.png \
	f.png g.png h.png i.png j.png k.png \
	next.png prev.png

install-data-local:
	$(mkinstalldirs) $(DESTDIR)${imgsdir}
	ln -fs ../imgs   $(DESTDIR)${wwwdir}/thread/i

uninstall-local:
	rm -f $(DESTDIR)${wwwdir}/thread/i
