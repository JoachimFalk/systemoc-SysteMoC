//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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
#ifndef XILINX_EDK_RUNTIME
# include <iostream>
#else
# include <stdlib.h>
#endif

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_tt.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>

#include <cmath>
#include <cassert>

#include <vpc_api.hpp>
namespace VC = SystemC_VPC::Config;

using namespace std;

// Maximum (and default) number of Src iterations. Lower default number via
//  command line parameter.
const int NUM_MAX_ITERATIONS = 1000000;


class Src: public smoc_actor {
public:
  smoc_port_out<double> out;
private:
  int i;
  
  void src() {
    std::cout << "src: " << i << " @ " << sc_time_stamp() << std::endl;

#ifndef NDEBUG
# ifndef XILINX_EDK_RUNTIME
#  ifndef VAST
    std::cout << "src: name: " << name() << " " << i  << std::endl;
#  else
    printf("src: %d\n", i);
#  endif
# else
    xil_printf("src: %u\n",i);
# endif
#endif
    out[0] = i++;
  }

  smoc_firing_state start;
public:
  Src(sc_module_name name, int from)
    : smoc_actor(name, start), i(from) {

      SMOC_REGISTER_CPARAM(from);

      start =
        (VAR(i) <= NUM_MAX_ITERATIONS) >>
        out(1)                         >>
        CALL(Src::src)                 >> start
      ;
  }
};

class Governor: public smoc_periodic_actor {
private:
  double utilization;
  std::string pMode;

  void changePowerMode(){
    if(pMode=="FAST"){
      pMode="SLOW";
      VC::changePowerMode(*this, pMode);
    }else{
      pMode="FAST";
      VC::changePowerMode(*this, pMode);
    }
  }

  bool check() const{
    if(pMode=="SLOW"){
      return utilization>0.7;
    }else{
      return utilization<0.2;
    }
  }

  smoc_firing_state start;

public:
  Governor(sc_module_name name)
      : smoc_periodic_actor( name, start,  sc_time(50, SC_MS), sc_time(50,SC_MS)),
        utilization(0.0), pMode("SLOW") {
    start =  GUARD(Governor::check)          >>
             CALL(Governor::changePowerMode) >> start;
    }
};

class Sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  void sink(void) {
    std::cout << "sink: name " << name() << " " << " @ " << sc_time_stamp() << std::endl;
#ifndef NDEBUG
# ifndef XILINX_EDK_RUNTIME
#  ifndef VAST
    std::cout << "sink: " << in[0] << std::endl;
#  else
    printf("sink: %f\n", in[0]);
#  endif
# else
    xil_printf("sink: %u\n",in[0]);
# endif
#endif
  }
  
  smoc_firing_state start;
public:
  Sink(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        in(1)             >>
        CALL(Sink::sink)  >>
        start
      ;
  }
};

class SqrRoot
: public smoc_graph {
public:
protected:
  Src       src;
  Sink      sink;
  Governor  governor;
public:
  SqrRoot( sc_module_name name, const int from = 1 )
    : smoc_graph(name),
      src("a1", from),
      sink("a2"),
      governor("a3") {
    connectNodePorts(src.out,sink.in);
  }
};

int sc_main (int argc, char **argv) {
  int from = 1;
  if (argc == 2) {
    const int iterations = atoi(argv[1]);
    assert(iterations < NUM_MAX_ITERATIONS);
    from = NUM_MAX_ITERATIONS - iterations;
  }
  smoc_top_moc<SqrRoot> sqrroot("sqrroot", from);
  sc_start();
  return 0;
}
