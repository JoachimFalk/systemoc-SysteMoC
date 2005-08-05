
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

#include "ib_m_atu.h"
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
 *  Pseudo Guard
 */
bool ib_m_atu::rq_guard() const {
  std::cout << "ATU> RQ-GUARD called!" << std::endl;
  return true;
}









/**
 * \brief writes stored message to MFETCH module
 *
 */
void ib_m_atu::to_mfetch()
{
  os << "ATU> forwarding message to MFETCH" << endl;

  //os << DBG_LOW << "ATU> forwarding message to MFETCH" << endl;
 
  /*
  // DEBUG output
  switch ( mfetch_msg->type ) {

    case tt_ib::TT_DATA :
      os << DBG_LOW << mfetch_msg->castme<tt_data>() << endl;
      break;
    
    case tt_ib::TT_RAW_HEADER :
      os << DBG_LOW << mfetch_msg->castme<tt_raw_header>() << endl;
      break;
   
    case tt_ib::TT_LLC_INFO :
      os << DBG_LOW << mfetch_msg->castme<tt_llc_info>() << endl;
      break;

    case tt_ib::TT_MESSAGE :
      os << DBG_LOW << mfetch_msg->castme<tt_message>() << endl;
      break;
      
    default:
      os << DBG_HIGH << "ATU> unknown TT type" << endl;
      assert(0);
  }
  */
  out_atu2mfetch[0] = mfetch_msg;
}


/**
 * \brief writes stored message to MSTORE module
 *
 */
void ib_m_atu::to_mstore()
{
  os << "ATU> forwarding message to MSTORE" << endl;
  //os << DBG_LOW << "ATU> forwarding message to MSTORE" << endl;
  
  /*
  // DEBUG output
  switch ( mstore_msg->type ) {

    case tt_ib::TT_DATA :
      os << DBG_LOW << mstore_msg->tt->castme<tt_data>() << endl;
      break;
    
    case tt_ib::TT_MESSAGE :
      os << DBG_LOW << mstore_msg->tt->castme<tt_raw_header>() << endl;
      break;
   
    default:
      os << DBG_HIGH << "ATU> unknown TT type" << endl;
      assert(0);
  }
  */
  out_atu2mstore[0] = mstore_msg;
}

/**
 *  stores messages received from BTHGRH_GEN module
 *  to a temporary variable in the module
 */
void ib_m_atu::store_bthgen_msg()
{
  os << "ATU> got message from BTH/GRH GEN. " << endl;
  //os << DBG_LOW  << "ATU> got message from BTH/GRH GEN. " << endl;
  
  mfetch_msg = in_bth_grh_gen2atu[0];
}


/**
 *  encapsulates TTs and adds source queue information,
 *  here RQ. message is stored in mstore_msg
 */
void ib_m_atu::store_rq_msg()
{
  os << "ATU> got message from RQ. " << endl;
  //os << DBG_LOW  << "ATU> got message from RQ. " << endl;
  
  mstore_msg = in_rq2atu[0];
}


/**
 *  encapsulates TTs and adds source queue information,
 *  here TQ. message is stored in mstore_msg
 */
void ib_m_atu::store_tq_msg()
{
  os << "ATU> got message from TQ. " << endl;
  //os << DBG_LOW  << "ATU> got message from TQ. " << endl;
  
  mstore_msg = in_tq2atu[0];
}

