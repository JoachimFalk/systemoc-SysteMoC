
/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * -----------------------------------------------------------------------------
 * ib_m_atu.h
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/


#ifndef IB_M_ATU_H
#define IB_M_ATU_H


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
//#include "ib_debug.h"
//#include "ib_channels_msgs.h"

#include <iostream>

// Oneof
#include <cosupport/oneof.hpp>

using Expr::isType;

// channel types
typedef CoSupport::oneof<tt_notification, tt_bo_notification, tt_data>   ct_queue2atu;
typedef CoSupport::oneof<tt_raw_header, tt_data>                         ct_bthgen2atu;

/**
 * \brief Address Translation Unit
 *
 * Since now there is no address translation functionality.
 * Incoming messages will be forwarded to appropriate
 * modules.
 */

class ib_m_atu : public smoc_actor {

  public:
    
    ///@name channels
    //@{
  
    // RQ -> ATU
    smoc_port_in<ct_queue2atu>    in_rq2atu;

    // TQ -> ATU
    smoc_port_in<ct_queue2atu>    in_tq2atu;

    // BTHGRH_GEN -> ATU
    smoc_port_in<ct_bthgen2atu>   in_bth_grh_gen2atu;
  
    // ATU -> MSTORE
    smoc_port_out<ct_queue2atu>   out_atu2mstore;
  
    // ATU -> MFETCH
    smoc_port_out<ct_bthgen2atu>  out_atu2mfetch;
    //@}

  private:
    
    /// all output from this module goes to this stream
    //dbg_ostream &os;
    ostream &os;

    // FSM states
    smoc_firing_state start;

// ATOMIC TRANSITIONS OPERATIONS
    
    /// forward msg from BTH/GRHGen module to MFETCH
    void forward_mfetch(); 
    
    /// forward msg from RQ module to MSTORE
    void forward_rq_mstore(); 
    
    /// forward msg from TQ module to MSTORE
    void forward_tq_mstore();

    /// process a TT_DATA of BTH/GHRHGen determined for MFETCH
    void process_bthgen_ttdata();
    
    /// process a TT_DATA of RQ determined for MSTORE
    void process_rq_ttdata();

    /// process a TT_DATA of TQ determined for MFETCH
    void process_tq_ttdata();

  public:
  
    ib_m_atu( sc_module_name name, ostream &os ) :
      smoc_actor( name, start ),
      os( os )
    {
    
// ******************************************************************** 
// ATU-TEST STATE MACHINE
//
// Any kind of processing is done in an singe transaction (=> "atomic")
//
// received TT_DATAs will be processed by ATU, other messages
// will be forwarded to appropriate module
// ********************************************************************

      start = ( in_rq2atu(1)
                && isType<tt_data>( in_rq2atu.getValueAt(0) ) )
              >> out_atu2mstore(1)
              >> call(&ib_m_atu::process_rq_ttdata)
              >> start
            | ( in_rq2atu(1)
                && !isType<tt_data>( in_rq2atu.getValueAt(0) ) )
              >> out_atu2mstore(1)
              >> call(&ib_m_atu::forward_rq_mstore)
              >> start
            | ( in_tq2atu(1)
                && isType<tt_data>( in_tq2atu.getValueAt(0) ) )
              >> out_atu2mstore(1)
              >> call(&ib_m_atu::process_tq_ttdata)
              >> start
            | ( in_rq2atu(1)
                && !isType<tt_data>( in_tq2atu.getValueAt(0) ) )
              >> out_atu2mstore(1)
              >> call(&ib_m_atu::forward_tq_mstore)
              >> start
            | ( in_bth_grh_gen2atu(1)
                && isType<tt_data>( in_bth_grh_gen2atu.getValueAt(0) ) )
              >> out_atu2mfetch(1)
              >> call(&ib_m_atu::process_bthgen_ttdata)
              >> start
            | ( in_bth_grh_gen2atu(1)
                && !isType<tt_data>( in_bth_grh_gen2atu.getValueAt(0) ) )
              >> out_atu2mfetch(1)
              >> call(&ib_m_atu::forward_mfetch)
              >> start;
    }
};

#endif // IB_M_ATU_H
