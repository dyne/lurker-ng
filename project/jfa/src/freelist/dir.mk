JFA_FREE_LIST_OBJS := $D/FreeList.lo
LIBJFA_OBJS        += $(JFA_FREE_LIST_OBJS)


$D/all:		$(JFA_FREE_LIST_OBJS)
$D/clean:
		rm -f $(JFA_FREE_LIST_OBJS)

$(JFA_FREE_LIST_OBJS:$D/%.lo=$D/.%.opts):	jfa/compile.opts
