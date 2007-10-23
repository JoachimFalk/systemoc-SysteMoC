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

#ifndef CRYPTO_MODULE_HPP
#define CRYPTO_MODULE_HPP

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include "dispatcher.hpp"
#include "merger.hpp"

#include "cryptoalgorithm.hpp"
#include "blowfish.hpp"
#include "des3.hpp"
#include "md5.hpp"

#include "examplenetworkpacket.hpp"

#define FIFO_SIZE 500

class CryptoModule : public smoc_graph{

  private:

    DES3 des;
    Blowfish bf;
    Dispatcher dispatcher;
    Merger merger;
    
  public:

    smoc_port_in< ExampleNetworkPacket > in;
    smoc_port_out< ExampleNetworkPacket > out;

    CryptoModule(sc_module_name name) 
      : smoc_graph(name),
        des("DES3"),
        bf("Blowfish"),
        dispatcher("Dispatcher"),
        merger("Merger")
        {

          // establish connection to interface ports
          dispatcher.in(in);
          merger.out(out);

          // establish connection btw actor in subgraph
          connectNodePorts<FIFO_SIZE>(dispatcher.out_des3, des.in);
          connectNodePorts<FIFO_SIZE>(dispatcher.out_blowfish, bf.in);
          
          connectNodePorts<FIFO_SIZE>(des.out, merger.in_des3);
          connectNodePorts<FIFO_SIZE>(bf.out, merger.in_blowfish);
          
        }
};

#endif // CRYPTO_MODULE_HPP_
