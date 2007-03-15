//  -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 sts=2 expandtab:
/*
 * Copyright (c) 2007 Hardware-Software-CoDesign, University of
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

#include <list>
#include <set>

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_moc.hpp>

#include "channels.hpp"

#include "Tupple_src.hpp"
#include "InvZrl.hpp"
#include "DcDecoding.hpp"
#include "InvZigZag.hpp"
#include "GenSnk.hpp"

class Testbench: public smoc_graph {
private:
  TuppleSrc tupple_src_i;
  InvZrl inv_zrl_i;  
  DcDecoding dc_decoder_i;
  InvZigZag invzigzag_i;
  m_gen_sink gen_sink_i;
public:
  Testbench(sc_module_name name, const std::string& tupple_input_file)
    : smoc_graph(name),
      tupple_src_i("mTuppleSource",tupple_input_file),
      inv_zrl_i("mInvZrl"),
      dc_decoder_i("mDcDecoder"),
      invzigzag_i("mInvZigZag"),
      gen_sink_i("mGenSink")
  { 
    connectNodePorts<1>(tupple_src_i.out,inv_zrl_i.in);
    connectNodePorts<1>(inv_zrl_i.out,dc_decoder_i.in);
    connectNodePorts<JPEG_BLOCK_SIZE>(dc_decoder_i.out,invzigzag_i.in);
    connectNodePorts<1>(invzigzag_i.out,gen_sink_i.in);
    
  }
};

#ifndef KASCPAR_PARSING
int sc_main (int argc, char **argv) {
  if (argc <= 1) {
    std::cerr
      << (argv[0] != NULL ? argv[0] : "???")
      << "<tupple_file>" << std::endl;
    exit(-1);
  }
  
  std::string tupple_file(argv[1]);


  smoc_top_moc<Testbench> testbench("testbench",tupple_file);
  
  sc_start(-1);
  
  return 0;
}
#endif
