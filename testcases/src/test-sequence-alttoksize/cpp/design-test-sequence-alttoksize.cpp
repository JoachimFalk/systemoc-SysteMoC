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

#include "smoc_synth_std_includes.hpp"

//template <class T>
class m_h_src: public smoc_actor {
public:
  smoc_port_out<double> out;
private:
  double i;
  
  void src_1() {
    std::cout << "src_1: " << i << std::endl;
    out[0]= i++;
  }
  void src_3() {
      for(unsigned int count = 0; count < 3; ++count)
      {
        std::cout << "src_3: " << i << std::endl;
        out[count]= i++;
      }
  }
  void src_5() {
      for(unsigned int count = 0; count < 5; ++count)
      {
        std::cout << "src_5: " << i << std::endl;
        out[count]= i++;
      }
  }

  smoc_firing_state start_1;
  smoc_firing_state start_3;
  smoc_firing_state start_5;

public:
  m_h_src(sc_core::sc_module_name name)
    : smoc_actor(name, start_1), i(1) {

    start_1 = out(1) >> SMOC_CALL(m_h_src::src_1) >> start_3;
    start_3 = out(3) >> SMOC_CALL(m_h_src::src_3) >> start_5;
    start_5 = out(5) >> SMOC_CALL(m_h_src::src_5) >> start_1;
  }
};

//template <class T> // actor type parameter T
class m_h_process: public smoc_actor {
public:
  smoc_port_in<double>  input;
  smoc_port_out<double> output;
private:
  
  // states of the firing rules state machine
  smoc_firing_state start_1;
  smoc_firing_state start_5;
  smoc_firing_state start_3;
  
  // action function for the firing rules state machine
  void doprocess_1() {
    output[0] = input[0] + 1000;
    std::cout << "process_1: " << output[0] << std::endl;
  }
  void doprocess_5() {
    for(unsigned int count = 0; count < 5; ++count)
    {
      output[count] = input[count] + 1000;
      std::cout << "process_5: " << output[count] << std::endl;
    }
  }
  void doprocess_3() {
    for(unsigned int count = 0; count < 3; ++count)
    {
      output[count] = input[count] + 1000;
      std::cout << "process_3: " << output[count] << std::endl;
    }
  }
public:
  m_h_process(
      sc_core::sc_module_name name//,        // name of actor
      //const vector<double> &taps  // the taps are the coefficients, starting
                                    // with the one for the most recent data item 
  ) : smoc_actor( name, start_1 )     //,

  {
    start_1 = input(1) >> output(1) >> SMOC_CALL(m_h_process::doprocess_1) >> start_5;
    start_5 = input(5) >> output(5) >> SMOC_CALL(m_h_process::doprocess_5) >> start_3;
    start_3 = input(3) >> output(3) >> SMOC_CALL(m_h_process::doprocess_3) >> start_1;
  }
};

//template <class T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  void sink_5(void) {
    for(unsigned int count = 0; count < 5; ++count)
    {
      std::cout << "sink_5: " << in[count] << std::endl;
    }
  }
  void sink_3(void) {
    for(unsigned int count = 0; count < 3; ++count)
    {
      std::cout << "sink_3: " << in[count] << std::endl;
    }
  }
  void sink_1(void) {
    std::cout << "sink_1: " << in[0] << std::endl;
  }
  
  smoc_firing_state start_5;
  smoc_firing_state start_3;
  smoc_firing_state start_1;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start_5) {
    start_5 = in(5) >> SMOC_CALL(m_h_sink::sink_5) >> start_3;
    start_3 = in(3) >> SMOC_CALL(m_h_sink::sink_3) >> start_1;
    start_1 = in(1) >> SMOC_CALL(m_h_sink::sink_1) >> start_5;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src  src;
  m_h_process  process;
  m_h_sink sink;
public:
  /*static vector<double> gentaps() {
    vector<double> retval;

    // vector [0,0,1]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(1);
    return retval;
  }*/
  
  m_h_top(sc_core::sc_module_name name)
    : smoc_graph(name),
      src("src"),
      process("process"),
      sink("sink") {

//    smoc_fifo<double> initialTokens(4);
//    initialTokens << 77 << 777 << 7777 << 7777;

//    connectNodePorts(src.out       , process.input, initialtokens);
    connectNodePorts<10>(src.out       , process.input);
    connectNodePorts<10>(process.output, sink.in);
  }
};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_h_top> top("top");
  sc_core::sc_start();
  return 0;
}
