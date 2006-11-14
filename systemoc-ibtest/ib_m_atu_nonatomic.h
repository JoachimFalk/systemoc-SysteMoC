/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

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
