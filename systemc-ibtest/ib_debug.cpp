/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * -----------------------------------------------------------------------------
 * ib_debug.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/
#include <fstream>
#include "ib_debug.h"

// DEBUG OUTPUT CONTROL

// USAGE:
// comment out the #define statement to disable module group's
// debug output

// MODULE GROUPS

// debug consumer interface (LIBHCA)
#define DBG_CONS          

// debug transport layer (TQ, RQ, AQ, WQM)
#define DBG_TL

// debug SCHED
#define DBG_SCHED        

// debug header processing units
#define DBG_HEADER_MODULES

// debug ports
#define DBG_PORTS

// debug virtual lanes
#define DBG_VL

// debug VL / QPC manager(s)
#define DBG_MAN           

// debug memory access modules (MSTORE, MFETCH, ATU)
#define DBG_MEM

// debug mfetch/mstore buffers
#define DBG_MEM_BUFFERS 

// debug FIFO modules (fifo2hca, hca2fifo)
#define DBG_FIFO

// debug sc_main
#define DBG_MAIN

//
// DEBUG OUTPUT STREAMS - for each group
//
// use it to define the debug level and the output colour
// of the module

#ifdef DBG_CONS
  dbg_ostream OUT_CONS(std::cout, DBG_MEDIUM, C_BLUE);
#else
  dbg_ostream OUT_CONS;
#endif

#ifdef DBG_TL
  dbg_ostream OUT_TL(std::cout, DBG_MEDIUM, C_RED);
#else
  dbg_ostream OUT_TL;
#endif

#ifdef DBG_SCHED
  dbg_ostream OUT_SCHED(std::cout, DBG_MEDIUM, C_RED);
#else
  dbg_ostream OUT_SCHED;
#endif

#ifdef DBG_HEADER_MODULES
  dbg_ostream OUT_HM(std::cout, DBG_MEDIUM, C_CYAN);
#else
  dbg_ostream OUT_HM;
#endif

#ifdef DBG_PORTS
  dbg_ostream OUT_PORT(std::cout, DBG_MEDIUM, C_MAGENTA);
#else
  dbg_ostream OUT_PORT;
#endif

#ifdef DBG_VL
  dbg_ostream OUT_VL(std::cout, DBG_HIGH, C_BLUE);
#else
  dbg_ostream OUT_VL;
#endif

#ifdef DBG_MAN
  dbg_ostream OUT_MAN(std::cout, DBG_HIGH, C_RED);
#else
  dbg_ostream OUT_MAN;
#endif

#ifdef DBG_MEM
  dbg_ostream OUT_MEM(std::cout, DBG_MEDIUM, C_GREEN);
#else
  dbg_ostream OUT_MEM;
#endif

#ifdef DBG_MEM_BUFFERS
  dbg_ostream OUT_MEM_BUFFER(std::cout, DBG_MEDIUM, C_GREEN);
#else
  dbg_ostream OUT_MEM_BUFFER;
#endif

#ifdef DBG_FIFO
  dbg_ostream OUT_FIFO(std::cout, DBG_MEDIUM, C_YELLOW);
#else
  dbg_ostream OUT_FIFO;
#endif

#ifdef DBG_MAIN
  dbg_ostream OUT_MAIN(std::cout, DBG_HIGH, C_NOCOLOR);
#else
  dbg_ostream OUT_MAIN;
#endif
