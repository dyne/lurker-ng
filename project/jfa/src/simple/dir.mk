JFA_TRANSACTION_OBJS := $D/SimplePromiseKeeper.lo \
                        $D/SimpleLockHolder.lo \
                        $D/SimpleTransactionManager.lo
LIBJFA_OBJS          += $(JFA_TRANSACTION_OBJS)


$D/all:		$(JFA_TRANSACTION_OBJS)
$D/clean:
		rm -f $(JFA_TRANSACTION_OBJS)

$(JFA_TRANSACTION_OBJS:$D/%.lo=$D/.%.opts):	jfa/compile.opts
