JFA_FTREE_OBJS := $D/FTree.lo
LIBJFA_OBJS    += $(JFA_FTREE_OBJS)


$D/all:		$(JFA_FTREE_OBJS)
$D/clean:
		rm -f $(JFA_FTREE_OBJS)

$(JFA_FTREE_OBJS:$D/%.lo=$D/.%.opts):	jfa/compile.opts
