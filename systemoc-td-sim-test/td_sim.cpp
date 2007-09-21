// vim: set sw=2 ts=8:
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

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <systemoc/smoc_pggen.hpp>
#endif


using Expr::field;

class td_source: public smoc_actor {
public:
  smoc_port_out<int> out_ramp;
private:
  int count;
  void process() {
    if(count>8)sc_stop();
    std::cerr << name() << " generating: ";
    out_ramp[0] = 42;
    count++;
    std::cerr << out_ramp[0] << std::endl;
  }
  
  smoc_firing_state start;
  
public:
  td_source( sc_module_name name ) :
    smoc_actor( name, start ),
    count(0)
  {
    start = out_ramp(1) >> call(&td_source::process) >> start;
  }
};


class td_ramp: public smoc_actor {
public:
  smoc_port_in<int> in;
private:
   
  void print() {
    std::cerr << name() << ": got token: ";
    std::cerr << in[0] << std::endl;
  }
    
  smoc_firing_state start;
  
public:
  td_ramp( sc_module_name name ) :
    smoc_actor( name, start )
  {
    start = in(1) >> CALL(td_ramp::print) >> start;
  }
};


class m_top : public smoc_graph {
public:
  m_top( sc_module_name name )
    : smoc_graph(name)
  {
    td_source      &src  = registerNode(new td_source("src"));
      
    td_ramp        &ramp = registerNode(new td_ramp("rmp"));
    connectNodePorts( src.out_ramp, ramp.in );
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
  sc_start();
  return 0;
}
