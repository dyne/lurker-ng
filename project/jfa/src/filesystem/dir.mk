JFA_FILESYSTEM_OBJS := $D/FileSystemVolume.lo $D/FileSystemVolumeManager.lo
LIBJFA_OBJS         += $(JFA_FILESYSTEM_OBJS)


$D/all:		$(JFA_FILESYSTEM_OBJS)
$D/clean:
		rm -f $(JFA_FILESYSTEM_OBJS)

$(JFA_FILESYSTEM_OBJS:$D/%.lo=$D/.%.opts):	jfa/compile.opts
