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
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>

template <typename T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<T> out;
private:
  T i;
  
  void src() {
    std::cout << name() << ": " << i << std::endl;
    out[0] = i++;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name)
    : smoc_actor(name, start),
      i(1) {
    start = out(1) >> call(&m_h_src::src) >> start;
  }
};

class m_h_srcbool: public smoc_actor {
public:
  smoc_port_out<int> out;
private:
  bool i;
  
  void src() {
    std::cout << name() << ": " << i << std::endl;
    out[0] = i ? 1 : 0; i = !i;
  }
  smoc_firing_state start;
public:
  m_h_srcbool(sc_core::sc_module_name name)
    : smoc_actor(name, start),
      i(false) {
    start = out(1) >> call(&m_h_srcbool::src) >> start;
  }
};

template <typename T>
class SelectX: public smoc_actor {
public:
  smoc_port_in<int>  Control;
  smoc_port_in<T>    Data0, Data1;
  smoc_port_out<T>   Output;
private:
  void action0() { 
    std::cout << "action0" << std::endl;
    Output[0] = Data0[0];
  }
  void action1() {
    std::cout << "action1" << std::endl;
    Output[0] = Data1[0];
  }
  smoc_firing_state atChannel0, atChannel1;
public:
  SelectX(sc_core::sc_module_name name, int initialChannel = 0)
    : smoc_actor(name, initialChannel ? atChannel1 : atChannel0) {
    atChannel0
      = (Control(1) && Data0(1) &&
         Control.getValueAt(0) == 0)  >>
        Output(1)                     >>
        call(&SelectX::action0)        >> atChannel0
      | (Control(1) && Data1(1) &&
         Control.getValueAt(0) == 1)  >>
        Output(1)                     >>
        call(&SelectX::action1)        >> atChannel1
      | Data0(1)                      >>
        Output(1)                     >>
        call(&SelectX::action0)        >> atChannel0;
    
    atChannel1
      = (Control(1) && Data0(1) &&
         Control.getValueAt(0) == 0)  >>
        Output(1)                     >>
        call(&SelectX::action0)        >> atChannel0
      | (Control(1) && Data1(1) &&
         Control.getValueAt(0) == 1)  >>
        Output(1)                     >>
        call(&SelectX::action1)        >> atChannel1
      | Data0(1)                      >>
        Output(1)                     >>
        call(&SelectX::action1)        >> atChannel1;
  }
};

template <typename T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<T> in;
private:
  int i;
  
  void sink(void) { std::cout << name() << ": " << in[0] << std::endl; }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> call(&m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_srcbool         srcbool;
  m_h_src<double>     src1, src2;
  SelectX<double>      select;
  m_h_sink<double>    sink;
public:
  m_h_top( sc_core::sc_module_name name )
    : smoc_graph(name),
      srcbool("srcbool"),
      src1("src1"), src2("src2"),
      select("select"),
      sink("sink") {
    connectNodePorts(srcbool.out, select.Control);
    connectNodePorts(src1.out, select.Data0);
    connectNodePorts(src2.out, select.Data1);
    connectNodePorts(select.Output, sink.in);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_h_top> top("top");
  sc_core::sc_start();
  return 0;
}
