
/******************************************************************************
 *                        Copyright 2004
 *                Lehrstuhl fuer Hardware-Software-Codesign
 *                  Universitaet Erlangen-Nuernberg
 *                    All rights reserved
 *
 * Title: InfiniBand HCA
 * Comment:
 * ----------------------------------------------------------------------------
 * ib_m_rethgen.cpp
 * ----------------------------------------------------------------------------
 * Modifications History:
 * ----------------------------------------------------------------------------
 * Notes:
 * ----------------------------------------------------------------------------
 * $log$
 *****************************************************************************/


#include <systemc.h>
#include "ib_m_rethgen.h"

/**
 *  \brief Forward incoming messages
 *
 *  Received messages of known type will be forwarded to the
 *  BTHGRH_GEN module.
 */

void ib_m_rethgen::forward()
{
  assert( in_tq2rethgen[0] != NULL );
  switch ( in_tq2rethgen[0]->type ) {
    
    case tt_ib::TT_PACKET_INFO :
      os << DBG_LOW << "RETHCHK> forwarding TT_PACKET_INFO to BTHGRH_GEN" << endl;
      out_rethgen2bth_grh_gen[0] = in_tq2rethgen[0];
      break;

    case tt_ib::TT_DATA :
      os << DBG_LOW << "RETHCHK> forwarding TT_DATA to BTHGRH_GEN" << endl;
      out_rethgen2bth_grh_gen[0] = in_tq2rethgen[0];
      break;

    default:
      os << DBG_HIGH << "RETHGEN> unexpected TT type" << endl;
      os << DBG_LOW << in_tq2rethgen[0];
      assert(0);
  }
}
