$D/all:		$D/vector $D/time
$D/clean:	$D
		rm -f $</*.o $</time $</vector

$D/time:	$D/time.o $D/foo.o

$D/.vector.link.opts:	jfa/link.opts
$D/.vector.opts:	jfa/compile.opts
$D/.time.link.opts:	jfa/link.opts
$D/.time.opts:	jfa/compile.opts
