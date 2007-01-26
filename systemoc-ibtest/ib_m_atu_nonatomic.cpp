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

#include "ib_m_atu_nonatomic.h"
using std::endl;


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

