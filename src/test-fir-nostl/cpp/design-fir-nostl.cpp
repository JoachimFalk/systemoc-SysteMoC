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
  
  void src() {
    std::cout << "src: " << i << std::endl;
    out[0]= i++;
  }
  smoc_firing_state start;
public:
  m_h_src(sc_core::sc_module_name name, unsigned int iterations)
    : smoc_actor(name, start), i(1) {
    
    SMOC_REGISTER_CPARAM(iterations);

    start = out(1) >> (SMOC_VAR(i) < iterations) >> SMOC_CALL(m_h_src::src) >> start;
  }
};

//template <class T> // actor type parameter T
class m_h_fir: public smoc_actor {
public:
  smoc_port_in<double>  input;
  smoc_port_out<double> output;
private:
  // taps parameter unmodifiable after actor instantiation
  double taps[3];
  // state information of the actor functionality
  double data[3];
  
  // states of the firing rules state machine
  smoc_firing_state start;
  smoc_firing_state write;
  
  // action function for the firing rules state machine
  void dofir() {
    // action [a] ==> [b] 
    const double &a = input[0];

    // T b := collect(zero(), plus, combine(multiply, taps, data))
    double b = 0;
    for ( unsigned int i = 0; i < 3; ++i ) {
      b += taps[i] * data[i];
    }
    output[0] = b;
    // data := [a] + [data[i] : for Integer i in Integers(0, #taps-2)];
    data[2] = data[1];
    data[1] = data[0];
    data[0] = a;
  }
public:
  m_h_fir(
      sc_core::sc_module_name name//,        // name of actor
      //const vector<double> &taps  // the taps are the coefficients, starting
                                    // with the one for the most recent data item 
  ) : smoc_actor( name, start )     //,

  {
    taps[0] = 0;
    taps[1] = 0;
    taps[2] = 1;
    data[0] = 0;
    data[1] = 0;
    data[2] = 0;

    start = input(1)             >>
            output(1)            >>
            SMOC_CALL(m_h_fir::dofir) >> start;
  }
};

//template <class T>
class m_h_sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  void sink(void) {
    std::cout << "sink: " << in[0] << std::endl;
  }
  
  smoc_firing_state start;
public:
  m_h_sink(sc_core::sc_module_name name)
    : smoc_actor(name, start) {
    start = in(1) >> SMOC_CALL(m_h_sink::sink) >> start;
  }
};

class m_h_top: public smoc_graph {
protected:
  m_h_src  src;
  m_h_fir  fir;
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
  
  m_h_top(sc_core::sc_module_name name, unsigned int iterations)
    : smoc_graph(name),
      src("src", iterations),
      fir("fir"/*, gentaps()*/),
      sink("sink") {

    smoc_fifo<double> initialTokens(4);
    initialTokens << 77 << 777 << 7777 << 7777;

    connectNodePorts(src.out   , fir.input, initialTokens);
    connectNodePorts(fir.output, sink.in);
  }
};

int sc_main (int argc, char **argv) {
  unsigned int iterations = 10000000; // ten million
  if (argc >= 2)
    iterations = atoi(argv[1]);
  smoc_top_moc<m_h_top> top("top", iterations);
  sc_core::sc_start();
  return 0;
}
