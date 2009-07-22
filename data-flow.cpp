// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of
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

static const char MESSAGE_HELLO[] = "Hello SysteMoC!";
static const char MESSAGE_ABC[]   = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
static const char MESSAGE_123[]   = "012345";

class Source: public smoc_actor {
public:
  smoc_port_out<char> out;
  Source(sc_module_name name, const char* msg) : smoc_actor(name, start),
    count(0), size(strlen(msg)), message(msg) {
    start = 
      GUARD(Source::hasToken)  >>
      out(1)                   >>
      CALL(Source::src)        >> start;
  }
private:
  smoc_firing_state start;

  unsigned int count, size;  // variables (functional state)
  const char* message;       //

  bool hasToken() const{ return count<size; } // guard
  void src() { out[0] = message[count++];  }  // action
};


template<typename T>
class Alternate: public smoc_actor {
public:
  smoc_port_in<T> in0, in1;
  smoc_port_out<T> out;
private:
  void forward0() {out[0] = in0[0];}
  void forward1() {out[0] = in1[0];}

  smoc_firing_state one, zero;
public:
  Alternate(sc_module_name name)
    : smoc_actor(name, one) {
    one =
      in0(1)    >>  out(1)      >>
      CALL(Alternate::forward0) >> zero;
    zero =
      in1(1)    >>  out(1)      >>
      CALL(Alternate::forward1) >> one;
  }
};


template<typename T>
class NDMerge: public smoc_actor {
public:
  smoc_port_in<T> in0, in1;
  smoc_port_out<T> out;
private:
  void forward0() {out[0] = in0[0];}
  void forward1() {out[0] = in1[0];}

  smoc_firing_state merge;
public:
  NDMerge(sc_module_name name)
    : smoc_actor(name, merge) {
    merge =
      in0(1)    >>  out(1)    >>
      CALL(NDMerge::forward0) >> merge
    |
      in1(1)    >>  out(1)    >>
      CALL(NDMerge::forward1) >> merge;
  }
};




class Sink: public smoc_actor {
public:
  // ports:
  smoc_port_in<char> in;
private:
  // actions:
  void sink() {
    std::cout << this->name() << " recv: \""
              << in[0] << "\"" << std::endl;
  }

  // FSM states:
  smoc_firing_state start;
public:
  // actor constructor
  Sink(sc_module_name name)
    : smoc_actor(name, start)
  {

    // FSM definition:
    start =
      in(1)                 >>
      CALL(Sink::sink) >> start
      ;
  }
};



class NetworkGraph: public smoc_graph {
protected:
  // actors
  Source           source0;
  Source           source1;
  //  Alternate<char>  alternate;
  NDMerge<char>    alternate;
  Sink             sink;
public:
  // networkgraph constructor
  NetworkGraph(sc_module_name name)
    : smoc_graph(name),
      // create actors
      source0("Source0", MESSAGE_123),
      source1("Source1", MESSAGE_ABC),
      alternate("Alternate"),
      sink("Sink")
  {
    connectNodePorts(source0.out, alternate.in0);
    connectNodePorts(source1.out, alternate.in1);
    connectNodePorts(alternate.out, sink.in);
  }
};

int sc_main (int argc, char **argv) {

  // create networkgraph
  smoc_top_moc<NetworkGraph> top("top");

  // start simulation (SystemC)
  sc_start();
  return 0;
}
