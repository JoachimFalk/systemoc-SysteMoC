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

#ifndef IOACTOR_HPP_
#define IOACTOR_HPP_

#include <iostream>

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_node_types.hpp>

#include "helper.hpp"
#include "examplenetworkpacket.hpp"

/**
 * Simple dummy actor to just foward packets through channel
 */
class IOActor : public smoc_actor{

  private:

    smoc_firing_state start;
    
    virtual void forwardPacket(){
      ExampleNetworkPacket packet;
      packet = in[0];

#ifdef CRYPTO_DEBUG    
      printMsg(packet);    
#endif //CRYPTO_DEBUG
      
      out[0] = packet;
    }
  
    void printMsg(ExampleNetworkPacket &packet){
      if(packet.processing_request != ExampleNetworkPacket::PR_set_key){

         std::cout << "ioactor " << this->basename() << "> Received new package with data:" << std::endl;
         std::cout << "\"";
         Helper::Datachars data;
         for(int i=0; i < packet.getUsedPayload(); i++){
           Helper::datawordToString(packet.payload[i], data);
           for(int j=0; j < BYTES_PER_DATAWORD; j++){
             std::cout << data.position[j];
           }
         }
         std::cout << "\"" <<  std::endl;
      } 
    }
     
  public:

    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out;

    IOActor(sc_module_name name) : smoc_actor(name, start){

      start = (in(1) && out(1)) >> CALL(IOActor::forwardPacket) >> start;

    }
};

#endif //IOACTOR_HPP_
