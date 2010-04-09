dnl Configuration fragment included by ../configure.in

AC_CONFIG_FILES([systemoc-sr/Makefile])
#subdirs="$subdirs systemoc-sr"
m4_sinclude([systemoc-sr/SimpleParityCheck/configure.in.frag])
m4_sinclude([systemoc-sr/matlab/configure.in.frag])
