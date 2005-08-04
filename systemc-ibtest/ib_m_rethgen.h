
/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * -----------------------------------------------------------------------------
 * ib_m_rethgen.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/


#ifndef IB_M_RETHGEN_H
#define IB_M_RETHGEN_H


// SysteMoC 2.0 Includes
#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

// InfiniBand Includes
#include "datatypes.h"
#include "tt_ib.h"
#include "ib_debug.h"



/**
 * \brief RETH generation module
 * 
 * It is a dummy module since now. It only takes incoming messages
 * and forwards them to the BTH/GRH generation module (ib_m_bth_grh_gen)
 */
class ib_m_rethgen : public smoc_actor {
  
  public:
  
    ///@name channels
    //@{
    // TQ -> RETHGEN
    smoc_port_in<tt_ib *> in_tq2rethgen;

    // RETHGEN -> BTHGRH_GEN
    smoc_port_out<tt_ib *> out_rethgen2bth_grh_gen;
    //@}

    
  private:
  
    /// all output from this module goes to this stream
    dbg_ostream &os;
 
    // FSM states
    smoc_firing_state start;
  
    /// forward incoming messages
    void forward();

  public:

    ib_m_rethgen( sc_module_name name, dbg_ostream &os ) :
      smoc_actor( name, start ),
      os(os)
    {
      start = in_tq2rethgen(1)
              >> out_rethgen2bth_grh_gen(1)
              >> call(&ib_m_rethgen::forward)
              >> start;
    }
};


#endif // IB_M_RETHGEN_H

