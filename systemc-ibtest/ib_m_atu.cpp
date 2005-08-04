
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
 * \brief writes stored message to MFETCH module
 *
 */
void ib_m_atu::to_mfetch()
{
  assert( mfetch_msg != NULL );

  os << DBG_LOW << "ATU> forwarding message to MFETCH" << endl;
  
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
  out_atu2mfetch[0] = mfetch_msg;
  mfetch_msg = NULL;
}


/**
 * \brief writes stored message to MSTORE module
 *
 */
void ib_m_atu::to_mstore()
{
  assert( mstore_msg != NULL );

  os << DBG_LOW << "ATU> forwarding message to MSTORE" << endl;
  
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
  out_atu2mstore[0] = mstore_msg;
  mstore_msg = NULL;
}

/**
 *  stores messages received from BTHGRH_GEN module
 *  to a temporary variable in the module
 */
void ib_m_atu::store_bthgen_msg()
{
  os << DBG_LOW  << "ATU> got message from BTH/GRH GEN. " << endl;
  
  assert( mfetch_msg == NULL );
  mfetch_msg = in_bth_grh_gen2atu[0];
}


/**
 *  encapsulates TTs and adds source queue information,
 *  here RQ. message is stored in mstore_msg
 */
void ib_m_atu::store_rq_msg()
{
  os << DBG_LOW  << "ATU> got message from RQ. " << endl;
  
  assert( mstore_msg == NULL );
  mstore_msg = new ct_atu2mstore( in_rq2atu[0], RQ );
}


/**
 *  encapsulates TTs and adds source queue information,
 *  here TQ. message is stored in mstore_msg
 */
void ib_m_atu::store_tq_msg()
{
  os << DBG_LOW  << "ATU> got message from TQ. " << endl;
  
  assert( mstore_msg == NULL );
  mstore_msg = new ct_atu2mstore( in_tq2atu[0], TQ );
}

