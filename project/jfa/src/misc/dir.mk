JFA_PUBLIC_OBJS := $D/Exception.lo \
                   $D/Environment.lo
LIBJFA_OBJS     += $(JFA_PUBLIC_OBJS)


$D/all:		$(JFA_PUBLIC_OBJS)
$D/clean:
		rm -f $(JFA_PUBLIC_OBJS)

$(JFA_PUBLIC_OBJS:$D/%.lo=$D/.%.opts):	jfa/compile.opts
