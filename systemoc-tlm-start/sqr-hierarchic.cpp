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

using namespace std; 

// Maximum (and default) number of Src iterations. Lower default number via
//  command line parameter.
const int NUM_MAX_ITERATIONS = 1000000;


/******************************************************************************
 *
 */
class Src: public smoc_actor {
public:
  smoc_port_out<double> out;
private:
  int i;
  
  void src() {
#ifndef NDEBUG
    cout << "src: " << i << std::endl;
#endif
    out[0] = i++;
  }
  
  smoc_firing_state start;
public:
  Src(sc_module_name name, SMOC_ACTOR_CPARAM(int,from))
    : smoc_actor(name, start), i(from) {
    start =
        (VAR(i) <= NUM_MAX_ITERATIONS) >>
        out(1)                         >>
        CALL(Src::src)                 >> start
      ;
  }
};


/******************************************************************************
 * Definition of the SqrLoop actor class
 */
class SqrLoop
  // All actor classes must be derived
  // from the smoc_actor base class
  : public smoc_actor {
public:
  // Declaration of input and output ports
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1, o2;
private:
  // Declaration of the actor functionality
  // via member variables and methods
  double tmp_i1;
  
  // action functions triggered by the
  // FSM declared in the constructor
  void copyStore()  { o1[0] = tmp_i1 = i1[0];  }
  void copyInput()  { o1[0] = tmp_i1;          }
  void copyApprox() { o2[0] = i2[0];           }
  
  // guard  functions used by the
  // FSM declared in the constructor
  bool check() const {
#ifndef NDEBUG
    cout << "check: " << tmp_i1 << ", " << i2[0] << std::endl;
#endif
    return fabs(tmp_i1 - i2[0]*i2[0]) < 0.0001;
  }
  
  // Declaration of firing states for the FSM
  smoc_firing_state start;
  smoc_firing_state loop;
public:
  // Constructor responsible for declaring the
  // communication FSM and initializing the actor
  SqrLoop(sc_module_name name)
    : smoc_actor( name, start ) {
    start =
        i1(1)                               >>
        o1(1)                               >>
        CALL(SqrLoop::copyStore)            >> loop
      ;
    loop  =
        (i2(1) &&  GUARD(SqrLoop::check))   >>
        o2(1)                               >>
        CALL(SqrLoop::copyApprox)           >> start
      | (i2(1) && !GUARD(SqrLoop::check))   >>
        o1(1)                               >>
        CALL(SqrLoop::copyInput)            >> loop
      ;
  }
};


/******************************************************************************
 *
 */
class Approx: public smoc_actor {
public:
  smoc_port_in<double>  i1, i2;
  smoc_port_out<double> o1;
private:
  // Square root successive approximation step of Newton
  void approx(void) { o1[0] = (i1[0] / i2[0] + i2[0]) / 2; }
  
  smoc_firing_state start;
public:
  Approx(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        (i1(1) && i2(1))         >>
        o1(1)                    >>
        CALL(Approx::approx)     >> start
      ;
  }
};


/******************************************************************************
 *
 */
class Dup: public smoc_actor {
public:
  smoc_port_in<double>  i1;
  smoc_port_out<double> o1, o2;

private:
  void dup() { 
    double in = i1[0];
    o1[0] = in;
    o2[0] = in;
  }
  
  smoc_firing_state start;
public:
  Dup(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        i1(1)                    >>
        (o1(1) && o2(1))         >>
        CALL(Dup::dup)           >> start
      ;
  }
};


/******************************************************************************
 *
 */
class Sink: public smoc_actor {
public:
  smoc_port_in<double> in;
private:
  void sink(void) {
#ifndef NDEBUG
    cout << "sink: " << in[0] << std::endl;
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


/******************************************************************************
 *
 */
class SqrConProd :
  public smoc_graph
{
private:
  Src      src;
  Sink     sink;

public:
  smoc_port_in<double>  &in;
  smoc_port_out<double> &out;

  SqrConProd(sc_module_name name, const int from = 1) :
    smoc_graph(name),
    src("a1", from),
    sink("a5"),
    in(sink.in),
    out(src.out)
  {}
};


/******************************************************************************
 *
 */
class SqrCalc :
  public smoc_graph
{
private:
  SqrLoop  sqrloop;
  Approx   approx;
  Dup      dup;

public:
  smoc_port_in<double>  &in;
  smoc_port_out<double> &out;

  SqrCalc(sc_module_name name) :
    smoc_graph(name),
    sqrloop("a2"),
    approx("a3"),
    dup("a4"),
    in(sqrloop.i1),
    out(sqrloop.o2)
  {
    connectNodePorts(sqrloop.o1, approx.i1);
#ifndef KASCPAR_PARSING
    connectNodePorts(approx.o1,  dup.i1,
                     smoc_fifo<double>(1));
    connectNodePorts(dup.o1,     approx.i2,
                     smoc_fifo<double>() << 2 );
#endif
    connectNodePorts(dup.o2,     sqrloop.i2);
  }
}; 


/******************************************************************************
 *
 */
class SqrRoot :
  public smoc_graph
{
public:
  SqrRoot(sc_module_name name, const int from = 1) :
    smoc_graph(name),
    sqrConProd("SqrConProd", from),
    sqrCalc("SqrCalc")
  {
    connectNodePorts(sqrConProd.out, sqrCalc.in);
    connectNodePorts(sqrCalc.out, sqrConProd.in);
  }

private:
  SqrConProd sqrConProd;
  SqrCalc    sqrCalc;
};


/******************************************************************************
 *
 */
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
