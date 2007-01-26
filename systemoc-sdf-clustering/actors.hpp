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

#include <smoc_moc.hpp>
#include <smoc_port.hpp>
#include <smoc_fifo.hpp>
#include <smoc_node_types.hpp>
#ifndef __SCFE__
//# include <smoc_scheduler.hpp>
# include <smoc_pggen.hpp>
#endif

class A1: public smoc_actor {
public:
  smoc_port_in<int>  i1;
  smoc_port_out<int> o1;
  smoc_port_out<int> o2;
protected:
  void copyToO1()
    { o1[0] = i1[0]; }
  void copyToO2()
    { o2[0] = i1[0]; }

  bool isOdd() const
    { return i1[0] & 1; }

  smoc_firing_state start;
public:
  A1(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        (i1(1) &&  GUARD(A1::isOdd))>>
        o1(1)                       >>
        CALL(A1::copyToO1)          >> start
      | (i1(1) && !GUARD(A1::isOdd))>>
        o2(1)                       >>
        CALL(A1::copyToO2)          >> start
      ;
  }
};

class A2: public smoc_actor {
public:
  smoc_port_in<int>  i1;
  smoc_port_out<int> o1;
  smoc_port_out<int> o2;
protected:
  void action()
    { o1[0] = i1[0]; o2[0] = i1[0]; }

  smoc_firing_state start;
public:
  A2(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        i1(1)                       >>
        (o1(1) && o2(1))            >>
        CALL(A2::action)            >> start
      ;
  }
};

class A3: public smoc_actor {
public:
  smoc_port_in<int>  i1;
  smoc_port_in<int>  i2;
  smoc_port_out<int> o1;
  smoc_port_out<int> o2;
protected:
  void action() {
    o1[0] = i1[0] + i2[0];
    o1[1] = i1[1] - i2[1];
    o2[0] = i1[0] + i2[1];
    o2[1] = i1[1] - i2[0];
  }

  smoc_firing_state start;
public:
  A3(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        (i1(2) && i2(2))            >>
        (o1(2) && o2(2))            >>
        CALL(A3::action)            >> start
      ;
  }
};

class A4: public smoc_actor {
public:
  smoc_port_in<int>  i1;
  smoc_port_in<int>  i2;
  smoc_port_out<int> o1;
protected:
  void action() {
    o1[0] = i1[0] * i2[0];
  }

  smoc_firing_state start;
public:
  A4(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        (i1(1) && i2(1))            >>
        o1(1)                       >>
        CALL(A4::action)            >> start
      ;
  }
};

class A5: public smoc_actor {
public:
  smoc_port_in<int>  i1;
  smoc_port_in<int>  i2;
  smoc_port_out<int> o1;
protected:
  void copyFromI1()
    { o1[0] = i1[0]; }
  void copyFromI2()
    { o1[0] = i2[0]; }

  smoc_firing_state start;
public:
  A5(sc_module_name name)
    : smoc_actor(name, start) {
    start =
        i1(1)                       >>
        o1(1)                       >>
        CALL(A5::copyFromI1)        >> start
      | i2(1)                       >>
        o1(1)                       >>
        CALL(A5::copyFromI2)        >> start
      ;
  }
};
