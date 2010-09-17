/* vim: set sw=2 ts=8 syn=c: */

#ifndef _INCLUDED_SMOC_CONFIG_H
#define _INCLUDED_SMOC_CONFIG_H

/* Defined to SysteMoC version */
#define SYSTEMOC_VERSION "0.9"
#define SYSTEMOC_MAJOR_VERSION "0"
#define SYSTEMOC_MINOR_VERSION "9"
#define SYSTEMOC_MICRO_VERSION "0"

/* Defined if debug support is enabled */
#undef SYSTEMOC_ENABLE_DEBUG

/* Defined if SGX support is enabled */
#undef SYSTEMOC_ENABLE_SGX

/* Defined if transition tracing support is enabled */
#undef SYSTEMOC_ENABLE_TRANSITION_TRACE

/* Defined if systemoc tracing support is enabled */
#undef SYSTEMOC_ENABLE_DATAFLOW_TRACE

/* Defined if hooking infrastructure is provided */
#undef SYSTEMOC_ENABLE_HOOKING

/* Define if you want SystemC-VPC Support */
#define SYSTEMOC_ENABLE_VPC

/* Define if you want WSDF Support */
#undef SYSTEMOC_ENABLE_WSDF

#if defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE) || defined(SYSTEMOC_ENABLE_TRANSITION_TRACE) || defined(SYSTEMOC_ENABLE_SGX)
# define SYSTEMOC_NEED_IDS
#endif

#if defined(SYSTEMOC_ENABLE_VPC)
# define PORT_ACCESS_COUNTER
#endif

#if defined(SYSTEMOC_DEBUG) && defined(SYSTEMOC_ENABLE_VPC)
# define SYSTEMOC_DEBUG_VPC_IF
#endif

#endif /* _INCLUDED_SMOC_CONFIG_H */