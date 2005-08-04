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

#include "dbg_ostream.h"

/**
 *  \file DEBUG output streams for HCA
 *
 * defines available debug output streams (color, priority)
 * and its MACRO names used in ib_m_hca.h and ib_sim_mod.h
 *
 */


// debug ostreams used by hca modules

/// Debug output stream for consumer
extern dbg_ostream OUT_CONS;

/// Debug output stream for TransportLayer modules
extern dbg_ostream OUT_TL;

/// Debug output stream for scheduler
extern dbg_ostream OUT_SCHED;

/// Debug output stream for header processing modules
extern dbg_ostream OUT_HM;

/// Debug output stream for send - and receive port
extern dbg_ostream OUT_PORT;

/// Debug output stream for virtual lanes
extern dbg_ostream OUT_VL; 

/// Debug output stream for VL - and QPC manager
extern dbg_ostream OUT_MAN;

/// Debug output stream for mfetch and mstore
extern dbg_ostream OUT_MEM;

/// Debug output stream for mfethc/mstore buffers
extern dbg_ostream OUT_MEM_BUFFER;

/// Debug output stream for fifo modules
extern dbg_ostream OUT_FIFO;

/// Debug output stream for sc_main function
extern dbg_ostream OUT_MAIN;


#ifdef MODES_EVALUATOR
#define NOT_DUMP_PAYLOAD
#define NO_DEBUG_OUTPUT
#endif //MODES_EVALUATOR

