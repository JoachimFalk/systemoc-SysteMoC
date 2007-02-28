
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
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

// InfiniBand Includes
#include "datatypes.h"
#include "tt_ib.h"
//#include "ib_debug.h"
//#include "ib_channels_msgs.h"

#include <iostream>

// Oneof
#include <cosupport/oneof.hpp>

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

    // message buffers for output channels
    ct_bthgen2atu   mfetch_msg;
    ct_queue2atu    mstore_msg;

    // FSM states
    smoc_firing_state start;
    smoc_firing_state have_mfetch_msg;
    smoc_firing_state have_mstore_msg;
    smoc_firing_state have_both_msgs;
    
// NON ATOMIC TRANSITIONS OPERATIONS
    
    // write message on output channel to MFETCH
    void to_mfetch();
    
    /// write message on output channel to MSTORE
    void to_mstore();
 
    /// stores an RQ message to the temp variable mstore_msg
    void store_bthgen_msg();
    
    /// stores an RQ message to the temp variable mstore_msg
    void store_rq_msg();

    /// stores an TQ message to the temp variable mstore_msg
    void store_tq_msg();


  public:
  
    ib_m_atu( sc_module_name name, ostream &os ) :
      smoc_actor( name, start ),
      os( os )
    {
              
// *****************************************************
// ATU-TEST STATE MACHINE
// USING TWO TRANSITIONS AND A MODULE BUFFER TO PROCESS
// AN INPUT TOKEN
//
// IN THIS EXAMPLE ALL MESSAGES WILL BE SIMPLY FORWARDED
// 
// *****************************************************

      // transitions from START state
      start = in_rq2atu(1)
              >> call(&ib_m_atu::store_rq_msg)
              >> have_mstore_msg
            | in_tq2atu(1)
              >> call(&ib_m_atu::store_tq_msg)
              >> have_mstore_msg
            | in_bth_grh_gen2atu(1)
              >> call(&ib_m_atu::store_bthgen_msg)
              >> have_mfetch_msg;
      
      // transitions from HAVE_MSTORE_MSG state
      have_mstore_msg = out_atu2mstore(1)
                        >> call(&ib_m_atu::to_mstore)
                        >> start
                      | in_bth_grh_gen2atu(1)
                        >> call(&ib_m_atu::store_bthgen_msg)
                        >> have_both_msgs;

      // transitions from HAVE_MFETCH_MSG state
      have_mfetch_msg = out_atu2mfetch(1)
                        >> call(&ib_m_atu::to_mfetch)
                        >> start
                      | in_rq2atu(1)
                        >> call(&ib_m_atu::store_rq_msg)
                        >> have_both_msgs
                      | in_tq2atu(1)
                        >> call(&ib_m_atu::store_tq_msg)
                        >> have_both_msgs;
      
      // transitions from HAVE_BOTH_MSGS state
      have_both_msgs =  out_atu2mfetch(1)
                        >> call(&ib_m_atu::to_mfetch)
                        >> have_mstore_msg
                      | out_atu2mstore(1)
                        >> call(&ib_m_atu::to_mstore)
                        >> have_mfetch_msg;
    }
};

#endif // IB_M_ATU_H
