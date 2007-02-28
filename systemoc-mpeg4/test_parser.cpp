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

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <stdlib.h>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif

#include <callib.hpp>

#include "block_parser.hpp"

#define INAMEblk "test_in.bit"
#define ONAMEblk "test_out.dat"

class m_source_parser: public smoc_actor {
  public:
    smoc_port_out<int> out;
  
  private:
    bool isStreamGood;
    int  ch;
    
    std::ifstream i1; 
    
    void process() {
      assert( !i1.eof() );
      out[0] = ch;
      std::cout << name() << "  write " << ch << std::endl;
      ch = i1.get(); isStreamGood = !i1.eof() && i1.good();
    }
    
    smoc_firing_state start;
  public:
    m_source_parser( sc_module_name name ) 
      :smoc_actor( name, start ) {
      i1.open(INAMEblk, ios::binary);
      
      ch = i1.get();
      
      isStreamGood = !i1.eof() && i1.good();
      
      start = (out.getAvailableSpace() >= 1 &&
               var(isStreamGood))
              >> CALL(m_source_parser::process)
              >> start;
    }
  ~m_source_parser( ){
        i1.close();
  }
};

class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:
    //std::ofstream fo; 
    
    void process() {
      std::cout << name() << " receiving " << in[0] << std::endl;
     // fo << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  public:
    m_sink( sc_module_name name )
      : smoc_actor( name, start )/*,
        fo(ONAMEblk)*/ {
      start = in(8) >> CALL(m_sink::process)  >> start;
    }
    
    ~m_sink() {
      //fo.close();
    
    }
};

class m_list_sink: public smoc_actor {
  public:
    smoc_port_in<cal_list<int>::t> in;
  private:
    void process() {
      std::cout << name() << " receiving " << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  public:
    m_list_sink( sc_module_name name )
      :smoc_actor( name, start ) {
      start = in(64) >> CALL(m_list_sink::process)  >> start;
    }
};

class PARSER_TEST
: public smoc_graph {
private:
  m_source_parser src_parser;
  m_block_parser  blparser;
  m_sink        snk0;
  m_list_sink   snk1;
  m_sink        snk2;
  m_sink        snk3;
public:
  
  
   PARSER_TEST( sc_module_name name )
    : smoc_graph(name),
      src_parser("src_parser"),
      blparser("blparser"),
      snk0("snk0"), 
      snk1("snk1"), 
      snk2("snk2"),
      snk3("snk3"){

    connectNodePorts( src_parser.out, blparser.I, smoc_fifo<int>(256));
    connectNodePorts( blparser.O0, snk0.in, smoc_fifo<int>(256));
    connectNodePorts( blparser.O1, snk1.in, smoc_fifo<cal_list<int>::t >(256));
    connectNodePorts( blparser.O2, snk2.in, smoc_fifo<int>(256));
    connectNodePorts( blparser.O3, snk3.in, smoc_fifo<int>(256));
      }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<PARSER_TEST> top("top");
  sc_start(-1);
  return 0;
}
