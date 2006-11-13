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

#ifndef DISPATCHER_HPP_
#define DISPATCHER_HPP_

/**
 * Dispatches packets to their corresponding encryption actors.
 * Currently 2 types are known Blowfish, DES3
 */
class Dispatcher: public smoc_actor{

  public:

    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out_blowfish, out_des3; 

  private:

    void dispatchBlowfish(){
#ifdef EX_DEBUG
      std::cout << "dispatcher> dispatching to blowfish" << std::endl;
#endif
      out_blowfish[0] = in[0];
    }

    void dispatchDES(){
#ifdef EX_DEBUG
      cout << "dispatcher> dispatching to DES3" << endl;
#endif
      out_des3[0] = in[0];
    }
    
    /**
     * guard used to determine which type of packet has to be dispatched
     */
    bool isType(ExampleNetworkPacket::EncryptionAlgorithm type) const{
      return (in[0].encryption_algorithm == type);
    }
    
    smoc_firing_state start;
    
  public:

    Dispatcher(sc_module_name name) : smoc_actor(name, start){
      
      start = // transition 1: if data available and requested encoding blowfish
              (in(1) && GUARD(Dispatcher::isType)(ExampleNetworkPacket::EM_blowfish) && out_blowfish(1))
              >> CALL(Dispatcher::dispatchBlowfish) >> start
              // transition 2: id data available and requested encoding DES3
            | (in(1) && GUARD(Dispatcher::isType)(ExampleNetworkPacket::EM_des3) && out_des3(1))
               >> CALL(Dispatcher::dispatchDES) >> start;
      
    }

};

#endif // DISPATCHER_HPP_

