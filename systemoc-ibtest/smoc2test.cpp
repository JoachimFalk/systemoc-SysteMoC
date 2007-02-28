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

// InifiniBand Includes
#include "tt_ib.h"
#include <cosupport/oneof.hpp>

using Expr::field;
using Expr::isType;


typedef CoSupport::oneof<int, char *> test_ty;

class m_source: public smoc_actor {
  public:
    smoc_port_out<test_ty> out;
  private:
    int count;
        
    void process() {
      
      std::cout << name() << " generating token: ";
      out[0] = count % 2;
      std::cout << out[0] << std::endl;

      count++;
    }
    
    smoc_firing_state start;
  
  public:
    m_source( sc_module_name name ) :
      smoc_actor( name, start ),
      count(0)
    {
      start = out(1) >> call(&m_source::process) >> start;
    }
};

class m_dispatcher : public smoc_actor 
{

  
  public:
    smoc_port_in<test_ty> in;
    smoc_port_out<int> out1;
    smoc_port_out<int> out2;

  private:

    int temp;
    
    // FSM states
    smoc_firing_state start;
    smoc_firing_state work;
    
    // methods
    
    void copy() {
      std::cout << name() << ": COPY called: ";
      temp = in[0];
      std::cout  << temp << std::endl;
    }
    
    void process1() {
      std::cout << name() << ": writing input to output channel 0" << std::endl;
      out1[0] = in[0];
    }
    
    void process2() {
      std::cout << name() << ": writing input to output channel 1" << std::endl;
      out2[0] = in[0];
    }

    bool check() const {
      if ( temp > 0 ) return true;
      else return false;
    }
    
  public:
    m_dispatcher( sc_module_name name ) :
      smoc_actor( name, start )
    {
      // use temprorary variable to buffer an input token before it will be transmitted
      /*
      start = (in(1) && field(*in.getValueAt(0), &tt_ib::type) ==  tt_ib::TT_PACKET_INFO) >> call(&m_dispatcher::copy) >> work;
      start = in(1) >> call(&m_dispatcher::copy) >> work;  
      work  = (var(temp) == 0) >> out1(1) >> call(&m_dispatcher::process1) >> start
            | (var(temp) == 1) >> out2(1) >> call(&m_dispatcher::process2) >> start;
      */
      
      // example with input message type checking
      /*
      start = (in(1) && (isType<int>(in.getValueAt(0))))) >> out1(1) >> call(&m_dispatcher::process1) >> start
            | (in(1) && (in.getValueAt(0) == 1)) >> out2(1) >> call(&m_dispatcher::process2) >> start;
      */
      // atomic transitions checking for 0 or 1 value on input channel
      start = (in(1) && (in.getValueAt(0) == 0)) >> out1(1) >> call(&m_dispatcher::process1) >> start
            | (in(1) && (in.getValueAt(0) == 1)) >> out2(1) >> call(&m_dispatcher::process2) >> start;
    }
};


class m_sink: public smoc_actor {
  public:
    smoc_port_in<int> in;
  private:
   
    void print() {
      std::cout << name() << ": got token: ";
      std::cout << in[0] << std::endl;
    }
    
    smoc_firing_state start;
  
  public:
    m_sink( sc_module_name name ) :
      smoc_actor( name, start )
    {
      start = in(1) >> call(&m_sink::print) >> start;
    }
};


class m_top
: public smoc_graph {
  public:
    m_top( sc_module_name name )
      : smoc_graph(name)
    {
      m_source      &src  = registerNode(new m_source("src"));
      
      m_sink        &snk1 = registerNode(new m_sink("snk1"));
      m_sink        &snk2 = registerNode(new m_sink("snk2"));
      m_dispatcher  &disp = registerNode(new m_dispatcher("disp"));
      connectNodePorts( src.out, disp.in );
      connectNodePorts( disp.out1, snk1.in );
      connectNodePorts( disp.out2, snk2.in );
    }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top> top("top");
  
  sc_start(-1);
  return 0;
}
