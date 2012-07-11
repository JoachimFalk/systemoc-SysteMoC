dnl Configuration fragment included by ../configure.in

ACJF_CHECK_LIB_MAESTROMM
ACJF_CHECK_LIB_MAESTROIL
ACJF_CHECK_LIB_MAESTROML

AC_CONFIG_FILES([
  Maestro-transceiver/Makefile
])
subdirs="$subdirs Maestro-transceiver"
