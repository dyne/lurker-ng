SUBDIRS = libesort common index prune render ui imgs .
EXTRA_DIST = FAQ

# Work around mimelib bug on MacOS X
AM_CPPFLAGS = -DDW_UNIX

# Support building mimelib ourselves
if LOCAL_MIMELIB
noinst_LIBRARIES = mimelib/libmimelib.a
nodist_mimelib_libmimelib_a_SOURCES = \
	mimelib/address.cpp mimelib/addrlist.cpp mimelib/attach.cpp \
	mimelib/basicmsg.cpp mimelib/binhex.cpp mimelib/body.cpp \
	mimelib/bodypart.cpp mimelib/boyermor.cpp \
	mimelib/datetime.cpp mimelib/disptype.cpp mimelib/dw_cte.cpp \
	mimelib/dw_date.cpp mimelib/dw_mime.cpp mimelib/dwstring.cpp \
	mimelib/entity.cpp mimelib/field.cpp mimelib/fieldbdy.cpp \
	mimelib/group.cpp mimelib/headers.cpp mimelib/mailbox.cpp \
	mimelib/mboxlist.cpp mimelib/mechansm.cpp \
	mimelib/mediatyp.cpp mimelib/message.cpp mimelib/msgcmp.cpp \
	mimelib/msgid.cpp mimelib/multipar.cpp mimelib/param.cpp \
	mimelib/text.cpp mimelib/token.cpp mimelib/uuencode.cpp
endif

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
	$(mkinstalldirs) $(DESTDIR)$(default_config_dir)
	$(INSTALL_DATA) lurker.conf apache.conf $(DESTDIR)$(default_config_dir)
	@echo
	@echo "Now put 'include $(default_config_dir)/apache.conf' in your apache config"
