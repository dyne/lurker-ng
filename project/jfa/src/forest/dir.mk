JFA_FOREST_OBJS := $D/ForestIterator.lo
LIBJFA_OBJS     += $(JFA_FOREST_OBJS)


$D/all:		$(JFA_FOREST_OBJS)
$D/clean:
		rm -f $(JFA_FOREST_OBJS)

$(JFA_FOREST_OBJS:$D/%.lo=$D/.%.opts):	jfa/compile.opts
