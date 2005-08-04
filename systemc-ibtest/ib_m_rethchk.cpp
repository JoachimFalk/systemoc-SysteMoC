
/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * ----------------------------------------------------------------------------
 * ib_m_rethchk.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/


#include "ib_m_rethchk.h"


/**
 *  \brief Forward incoming messages
 *
 *  Received TT_RAW_DWORD messages will be deleted, because since
 *  now there is no RDMA support.
 *  All other messages of appropriate type will be forwarded
 *  to the Receive Queue module.
 */

void ib_m_rethchk::forward()
{
  assert( in_bth_grh_chk2rethchk[0] != NULL );
  switch ( in_bth_grh_chk2rethchk[0]->type ) {
    
    case tt_ib::TT_PACKET_INFO :
      os << DBG_LOW << "RETHCHK> forwarding TT_PACKET_INFO to RQ" << endl;
      out_rethchk2rq[0] = in_bth_grh_chk2rethchk[0];
      break;

    case tt_ib::TT_RAW_DWORD :
      os << DBG_LOW << "RETHCHK> received TT_RAW_DWORD" << endl;
      // no rdma operations -> ignore raw data
      DELETE( in_bth_grh_chk2rethchk[0] ); 
      break;
    
    case tt_ib::TT_MESSAGE :
      os << DBG_LOW << "RETHCHK> TT_MESSAGE received" << endl;
      out_rethchk2rq[0] = in_bth_grh_chk2rethchk[0];
      break;
      
    default:
      os << DBG_HIGH << "RETHCHK> unexpected TT type" << endl;
      os << DBG_LOW << in_bth_grh_chk2rethchk[0];
      assert(0);
  }
}
