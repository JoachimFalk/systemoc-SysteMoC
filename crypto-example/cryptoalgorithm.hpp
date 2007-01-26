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

#ifndef CRYPTOALGORITHM_HH
#define CRYPTOALGORITHM_HH

/**
  Written 2004 by Andreas Schallenberg
  Carl von Ossietzky Universität Oldenburg
  Andreas.Schallenberg@Uni-Oldenburg.de

  Translation from SystemC using OSSS to SysteMoC
  by Carsten Riedel
  */

#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_node_types.hpp>

#include "examplenetworkpacket.hpp"
#include "exectimelogger.hpp"

/**
 * Base class for encryption algorithms. Specifying state machine used
 * to model behaviour and interface of algorithm objects.
 */
class CryptoAlgorithm : public smoc_actor
{
  public:

    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out;
    
  private:
    
    smoc_firing_state start, processPacket, sendPacket;
    ExampleNetworkPacket packet;
    
    // io methods of actor
    void readPacket();
    void writePacket();
    // packet processing methods
    void encryptPacket();
    void decryptPacket();
    void initKey();
    // guard for checking processing request of read packet
    bool isRequest(ExampleNetworkPacket::ProcessingRequest request) const;
  
  public:
  
    CryptoAlgorithm(sc_module_name name);
    virtual ~CryptoAlgorithm();
    
    // used to initialize key if required
    virtual void setKey(ExampleNetworkPacket packet);
    
    virtual void setKeyBits(sc_uint<3> part, sc_bv<56> bits, sc_uint<3> used_bytes_in_key);
    virtual void initialize();
    virtual void encrypt64(sc_bv<64> & data);
    virtual void decrypt64(sc_bv<64> & data);
    virtual void encryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
    virtual void decryptUpTo128(sc_bv<128> & data, sc_uint<5> length_in_bytes);
};

#endif // CRYPTOALGORITHM_HH
