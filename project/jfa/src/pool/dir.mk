JFA_GUARDEDPOOL_OBJS := $D/GuardedPool.lo
LIBJFA_OBJS          += $(JFA_GUARDEDPOOL_OBJS)


$D/all:		$(JFA_GUARDEDPOOL_OBJS)
$D/clean:
		rm -f $(JFA_GUARDEDPOOL_OBJS)

$(JFA_GUARDEDPOOL_OBJS:$D/%.lo=$D/.%.opts):	jfa/compile.opts
