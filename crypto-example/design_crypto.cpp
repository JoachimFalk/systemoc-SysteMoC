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

#include <iostream>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

//#include "examplenetworkpacket.hh"

#include "producermodule.hpp"
#include "consumermodule.hpp"
#include "channel.hpp"

#include "cryptomodule.hpp"

#define FIFO_SIZE 500
/**
 * Actor-graph of crypto example
 */
class CryptoExample : public smoc_graph{

  private:

    ProducerModule pModule;
    ConsumerModule cModule;
    CryptoModule pEncModule, pDecModule, cEncModule, cDecModule;
    MD5 pMd5_sign, pMd5_check, cMd5_sign, cMd5_check;
    Channel channel;
    
  public:

    CryptoExample(sc_module_name name, const char* input, int run)
      : smoc_graph(name),
        pModule("producer", input, run),
        cModule("consumer"),
        pEncModule("pEncModule"),
        pDecModule("pDecModule"),
        cEncModule("cEncModule"),
        cDecModule("cDecModule"),
        pMd5_sign("pMd5_sign"),
        pMd5_check("pMd5_check"),
        cMd5_sign("cMd5_sign"),
        cMd5_check("cMd5_check"),
        channel("channel")
        {

          connectNodePorts<FIFO_SIZE>(pModule.out, pEncModule.in);
          connectNodePorts<FIFO_SIZE>(pEncModule.out, pMd5_sign.in);
          connectNodePorts<FIFO_SIZE>(pMd5_sign.out, channel.in1);
          connectNodePorts<FIFO_SIZE>(channel.out1, cMd5_check.in);
          connectNodePorts<FIFO_SIZE>(cMd5_check.out, cDecModule.in);
          connectNodePorts<FIFO_SIZE>(cDecModule.out, cModule.in);
       
          connectNodePorts<FIFO_SIZE>(cModule.out, cEncModule.in);
          connectNodePorts<FIFO_SIZE>(cEncModule.out, cMd5_sign.in);
          connectNodePorts<FIFO_SIZE>(cMd5_sign.out, channel.in2);
          connectNodePorts<FIFO_SIZE>(channel.out2, pMd5_check.in);
          connectNodePorts<FIFO_SIZE>(pMd5_check.out, pDecModule.in);
          connectNodePorts<FIFO_SIZE>(pDecModule.out, pModule.in);

        }
};

int sc_main (int argc, char **argv) {
  const char* input;
  int run=0;
  
  if(argc <= 1){
    std::cerr << "Please specify input file for producer!\n"
                 "simulation-crypto [input-file]" << std::endl;
  } 
  if (argc > 1) {
    input = argv[1];
  }
  if (argc > 2) {
    run = atoi(argv[2]);
  }
  smoc_top_moc<CryptoExample> cryptoexample("crypto_example", input, run);
  
  sc_start();
  return 0;
}

