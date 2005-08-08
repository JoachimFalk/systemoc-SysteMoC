
/*******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * -----------------------------------------------------------------------------
 * ib_m_atu.cpp
 * -----------------------------------------------------------------------------
 * Modifications History:
 * -----------------------------------------------------------------------------
 * Notes:
 * -----------------------------------------------------------------------------
 * $log$
 ******************************************************************************/

#include "ib_m_atu_atomic.h"
using std::endl;


/**
 * Forward incoming messages from BTH/GRHGen module to MFETCH
 */
void ib_m_atu::forward_mfetch() 
{
  std::cout << "ATU> forwarding BTH/GER Gen message to MFETCH" << std::endl;
  out_atu2mfetch[0] = in_bth_grh_gen2atu[0];
}


/**
 * Forward incoming messages from RQ module to MSTORE
 */
void ib_m_atu::forward_rq_mstore() 
{
  std::cout << "ATU> forwarding RQ message to MSTORE" << std::endl;
  out_atu2mstore[0] = in_rq2atu[0];
}


/**
 * Forward incoming messages from TQ module to MSTORE
 */
void ib_m_atu::forward_tq_mstore() 
{
  std::cout << "ATU> forwarding TQ message to MSTORE" << std::endl;
  out_atu2mstore[0] = in_tq2atu[0];
}

/**
 * process a TT_DATA of BTH/GRHGen determined for MFETCH
 */
void ib_m_atu::process_bthgen_ttdata() {
  std::cout << "ATU> processing TT_DATA(BTH/GRHGen) for MFETCH" << std::endl;
  out_atu2mfetch[0] = in_bth_grh_gen2atu[0];
}

/**
 * process a TT_DATA of RQ determined for MSTORE
 */
void ib_m_atu::process_rq_ttdata() {
  std::cout << "ATU> processing TT_DATA(RQ) for MSTORE" << std::endl;
  out_atu2mstore[0] = in_rq2atu[0];
}

/**
 * process a TT_DATA of TQ determined for MSTORE
 */
void ib_m_atu::process_tq_ttdata() {
  std::cout << "ATU> processing TT_DATA(TQ) for MSTORE" << std::endl;
  out_atu2mstore[0] = in_tq2atu[0];
}

