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

#include <iostream>
#include <cmath>

#include <systemc>

class Src: public sc_core::sc_module {
public:
  // Declaration of fifo output port
  sc_core::sc_fifo_out<double> out;
private:
  // SystemC thread process
  void src() {
    int i = 1;
    while (true) {
      std::cout << "src: " << i << std::endl;
      out.write(i++);
    }
  }
public:
  SC_HAS_PROCESS(Src);
  Src(sc_core::sc_module_name name)
      : sc_core::sc_module(name)
    { SC_THREAD(src); }
};

// Definition of the SqrLoop actor class
class SqrLoop: public sc_core::sc_module {
public:
  // Declaration of fifo input and output ports
  sc_core::sc_fifo_in<double>   i1, i2;
  sc_core::sc_fifo_out<double>  o1, o2;
private:
  // SystemC thread process
  void loop() {
    double tmp, res;
    while (true) {
      tmp = i1.read(); // get token from Src
      do { /* Do one approximation step */ 
           o1.write(tmp); res = i2.read();
      } while (/* Good enough? */
               std::fabs(tmp-res*res)>=0.0001);
      o2.write(res); // write token to Sink
    }
  }
public:
  SC_HAS_PROCESS(SqrLoop);
  SqrLoop(sc_core::sc_module_name name)
      : sc_core::sc_module(name)
    { SC_THREAD(loop); }
};

class Sink: public sc_core::sc_module {
public:
  // Declaration of fifo input port
  sc_core::sc_fifo_in<double> in;
private:
  // SystemC thread process
  void sink() {
    while (true) {
      double res = in.read();
      std::cout << "sink: " << res << std::endl;
    }
  }
public:
  SC_HAS_PROCESS(Sink);
  Sink(sc_core::sc_module_name name)
      : sc_core::sc_module(name)
    { SC_THREAD(sink); }
};

class Approx: public sc_core::sc_module {
public:
  sc_core::sc_fifo_in<double>   i1, i2;
  sc_core::sc_fifo_out<double>  o1;
private:
  // Square root successive approximation step of Newton
  void approx() {
    while (true) {
      double i1Val = i1.read();
      double i2Val = i2.read();
      o1.write((i1Val/i2Val+i2Val)/2);
    }
  }
public:
  SC_HAS_PROCESS(Approx);
  Approx(sc_core::sc_module_name name)
      : sc_core::sc_module(name)
    { SC_THREAD(approx); }
};

class Dup: public sc_core::sc_module {
public:
  sc_core::sc_fifo_in<double>  i1;
  sc_core::sc_fifo_out<double> o1, o2;
private:
  void dup() {
    while (true) {
      double val = i1.read();
      o1.write(val); o2.write(val);
    }
  }
public:
  SC_HAS_PROCESS(Dup);
  Dup(sc_core::sc_module_name name)
      : sc_core::sc_module(name)
    { SC_THREAD(dup); }
};

class SqrRoot: public sc_core::sc_module {
protected:
  Src      src;
  SqrLoop  sqrLoop;
  Approx   approx;
  Dup      dup;
  Sink     sink;

  sc_core::sc_fifo<double> fifoSrc2SqrLoop;
  sc_core::sc_fifo<double> fifoSqrLoop2Sink;
  sc_core::sc_fifo<double> fifoSqrLoop2Approx;
  sc_core::sc_fifo<double> fifoApprox2Dup;
  sc_core::sc_fifo<double> fifoDup2Approx;
  sc_core::sc_fifo<double> fifoDup2SqrLoop;
public:
  SqrRoot(sc_core::sc_module_name name)
    : sc_core::sc_module(name),
      src("src"),
      sqrLoop("sqrLoop"),
      approx("approx"),
      dup("dup"),
      sink("sink")
  {
    src.out(fifoSrc2SqrLoop); sqrLoop.i1(fifoSrc2SqrLoop);
    sqrLoop.o2(fifoSqrLoop2Sink); sink.in(fifoSqrLoop2Sink);
    sqrLoop.o1(fifoSqrLoop2Approx); approx.i1(fifoSqrLoop2Approx);
    approx.o1(fifoApprox2Dup); dup.i1(fifoApprox2Dup);
    dup.o1(fifoDup2Approx); approx.i2(fifoDup2Approx);
    dup.o2(fifoDup2SqrLoop); sqrLoop.i2(fifoDup2SqrLoop);
    fifoApprox2Dup.write(1); // Add one initial token.
  }
};

int sc_main (int argc, char **argv) {
  SqrRoot sqrRoot("sqrRoot");
  sc_core::sc_start();
  return 0;
}
