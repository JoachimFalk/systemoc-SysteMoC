
/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * -----------------------------------------------------------------------------
 * ib_m_rethchk.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/


#ifndef IB_M_RETHCHK_H
#define IB_M_RETHCHK_H

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
 * \brief RETH checking module
 *
 * It is a dummy module since now. It only takes incoming messages
 * and forwards them to the Receive Queue module (ib_m_recveive_queue)
 */

class ib_m_rethchk : public smoc_actor {

  public:
  
    ///@name channels
    //@{
    
    // BTHGRH_CHK -> RETHCHK
    smoc_port_in<tt_ib *> in_bth_grh_chk2rethchk;

    // RETHCHK -> RQ
    smoc_port_out<tt_ib *> out_rethchk2rq;
    //@}


  private:
  
    /// all output from this module goes to this stream
    dbg_ostream &os;
  
    // FSM states
    smoc_firing_state start;
  
    /// forward incoming messages
    void forward();
  
  public:

    ib_m_rethchk( sc_module_name name, dbg_ostream &os ) :
      smoc_actor( name, start ),
      os( os )
    {
      start = in_bth_grh_chk2rethchk(1)
              >> out_rethchk2rq(1)
              >> call(&ib_m_rethchk::forward)
              >> start;
    }
};


#endif // IB_M_RETHCHK_H

