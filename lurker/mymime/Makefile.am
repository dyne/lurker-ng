# Work around mimelib bug on MacOS X
AM_CPPFLAGS = -DDW_UNIX

# Support building mimelib ourselves
if LOCAL_MIMELIB
noinst_LIBRARIES = libmimelib.a
nodist_libmimelib_a_SOURCES = \
	../mimelib/address.cpp ../mimelib/addrlist.cpp ../mimelib/attach.cpp \
	../mimelib/basicmsg.cpp ../mimelib/binhex.cpp ../mimelib/body.cpp \
	../mimelib/bodypart.cpp ../mimelib/boyermor.cpp \
	../mimelib/datetime.cpp ../mimelib/disptype.cpp ../mimelib/dw_cte.cpp \
	../mimelib/dw_date.cpp ../mimelib/dw_mime.cpp ../mimelib/dwstring.cpp \
	../mimelib/entity.cpp ../mimelib/field.cpp ../mimelib/fieldbdy.cpp \
	../mimelib/group.cpp ../mimelib/headers.cpp ../mimelib/mailbox.cpp \
	../mimelib/mboxlist.cpp ../mimelib/mechansm.cpp \
	../mimelib/mediatyp.cpp ../mimelib/message.cpp ../mimelib/msgcmp.cpp \
	../mimelib/msgid.cpp ../mimelib/multipar.cpp ../mimelib/param.cpp \
	../mimelib/text.cpp ../mimelib/token.cpp ../mimelib/uuencode.cpp
endif
