JFA_VECTOR_OBJS := $D/Vector.lo
LIBJFA_OBJS        += $(JFA_VECTOR_OBJS)


$D/all:		$(JFA_VECTOR_OBJS)
$D/clean:
		rm -f $(JFA_VECTOR_OBJS)

$(JFA_VECTOR_OBJS:$D/%.lo=$D/.%.opts):	jfa/compile.opts
