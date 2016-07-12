/* src/systemoc/headers/systemoc/smoc_config.h.  Generated from smoc_config.h.in by configure.  */
/* vim: set sw=2 ts=8 syn=c: */

#ifndef _INCLUDED_SYSTEMOC_SMOC_CONFIG_H
#define _INCLUDED_SYSTEMOC_SMOC_CONFIG_H

/* Defined to SysteMoC version */
#define SYSTEMOC_VERSION "1.0"
#define SYSTEMOC_MAJOR_VERSION "1"
#define SYSTEMOC_MINOR_VERSION "0"
#define SYSTEMOC_MICRO_VERSION "0"

/* Defined if debug support is enabled */
#define SYSTEMOC_ENABLE_DEBUG yes

/* Defined if SGX support is enabled */
/* #undef SYSTEMOC_ENABLE_SGX */

/* Defined if transition tracing support is enabled */
#define SYSTEMOC_ENABLE_TRANSITION_TRACE yes

/* Defined if systemoc tracing support is enabled */
/* #undef SYSTEMOC_ENABLE_DATAFLOW_TRACE */

/* Defined if hooking infrastructure is provided */
/* #undef SYSTEMOC_ENABLE_HOOKING */

/* Defined if SystemC-VPC support is enabled */
/* #undef SYSTEMOC_ENABLE_VPC */

/* Defined if Maestro library support is enabled */
/* #undef SYSTEMOC_ENABLE_MAESTRO */

#if defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE) || defined(SYSTEMOC_ENABLE_TRANSITION_TRACE) || defined(SYSTEMOC_ENABLE_SGX)
# define SYSTEMOC_NEED_IDS
#endif //defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE) || defined(SYSTEMOC_ENABLE_TRANSITION_TRACE) || defined(SYSTEMOC_ENABLE_SGX)

#if defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)
# define SYSTEMOC_ENABLE_DEBUG yes
#endif //defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)

#if defined(SYSTEMOC_ENABLE_VPC)
# define SYSTEMOC_PORT_ACCESS_COUNTER
#endif //defined(SYSTEMOC_ENABLE_VPC)

#if defined(SYSTEMOC_DEBUG) && defined(SYSTEMOC_ENABLE_VPC)
# define SYSTEMOC_DEBUG_VPC_IF
#endif //defined(SYSTEMOC_DEBUG) && defined(SYSTEMOC_ENABLE_VPC)

// Pull config of Maestro libraries
#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/maestro_config.h>
#endif //SYSTEMOC_ENABLE_MAESTRO

#endif //_INCLUDED_SYSTEMOC_SMOC_CONFIG_H
