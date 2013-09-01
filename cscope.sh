mkdir -p cscope
FILE=$PWD/cscope/cscope.files
find "$PWD/src/"  \
	"$PWD/src/board/st/pdk7105"  \
	"$PWD/src/common" \
	"$PWD/src/cpu/sh" \
	"$PWD/src/cpu/sh/stx7105" \
	"$PWD/src/disk" \
	"$PWD/src/fs" \
	"$PWD/src/fs/cramfs" \
	"$PWD/src/fs/ext2" \
	"$PWD/src/fs/fat" \
	"$PWD/src/fs/fdos" \
	"$PWD/src/fs/jffs2" \
	"$PWD/src/fs/reiserfs" \
	"$PWD/src/include" \
	"$PWD/src/include/linux" \
	"$PWD/src/include/configs" \
	"$PWD/src/include/asm-sh" \
	"$PWD/src/lib_generic" \
	"$PWD/src/lib_sh" \
	"$PWD/src/nand_spl" \
	"$PWD/src/net" \
	-maxdepth 1  -type f -name "*.[ch]" -print | sort > $FILE
find "$PWD/src/drivers" -type f -name "*.[ch]" -print | sort >> $FILE

cd cscope
cscope -b -q -k
