LIBJFA_PFX := $D/libjfa
LIBJFA     := $D/libjfa.la

$D/all:		$(LIBJFA)
$D/clean:
		rm -f $(LIBJFA_PFX).so* $(LIBJFA)

$D/libjfa.la:	$(LIBJFA_OBJS)
