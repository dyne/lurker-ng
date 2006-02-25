SUBDIRS = mymime libesort common index prune render ui imgs .
EXTRA_DIST = FAQ

install-data-local:
	$(mkinstalldirs) $(DESTDIR)$(default_config_dir)
	@echo
	@echo "*** PLEASE READ ***"
	@echo
	@echo "Lurker config files have not been installed (for your protection)"
	@echo "If you would like the default config files installed, run:"
	@echo "  make install-config"
	@echo
	@echo "*** PLEASE READ ***"
	@echo

install-config:
	$(mkinstalldirs) $(DESTDIR)$(localstatedir)/lib/$(PACKAGE)
	if ! test -f $(DESTDIR)$(localstatedir)/lib/$(PACKAGE)/db; then \
		echo 1 8192 255 > $(DESTDIR)$(localstatedir)/lib/$(PACKAGE)/db; \
	fi
	$(mkinstalldirs) $(DESTDIR)$(default_config_dir)
	$(INSTALL_DATA) lurker.conf apache.conf $(DESTDIR)$(default_config_dir)
	@echo
	@echo "Now put 'include $(default_config_dir)/apache.conf' in your apache config"
	@echo "And don't forget to setup a cron job as described in the INSTALL file"
