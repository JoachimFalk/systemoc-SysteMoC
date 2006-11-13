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

