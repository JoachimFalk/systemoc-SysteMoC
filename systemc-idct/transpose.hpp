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

#ifndef _INCLUDED_TRANSPOSE_HPP
#define _INCLUDED_TRANSPOSE_HPP

#include <systemc.h>

#include "callib.hpp"

class m_transpose: public sc_module {
public:
  sc_fifo_in<int>  I0, I1, I2, I3, I4, I5, I6, I7;
  sc_fifo_out<int> O0, O1, O2, O3, O4, O5, O6, O7;
private:
  void action0() {
    O0.write(I0.read()); O0.write(I1.read()); O0.write(I2.read()); O0.write(I3.read());
    O0.write(I4.read()); O0.write(I5.read()); O0.write(I6.read()); O0.write(I7.read());
  }
  void action1() {
    O1.write(I0.read()); O1.write(I1.read()); O1.write(I2.read()); O1.write(I3.read());
    O1.write(I4.read()); O1.write(I5.read()); O1.write(I6.read()); O1.write(I7.read());
  }
  void action2() {
    O2.write(I0.read()); O2.write(I1.read()); O2.write(I2.read()); O2.write(I3.read());
    O2.write(I4.read()); O2.write(I5.read()); O2.write(I6.read()); O2.write(I7.read());
  }
  void action3() {
    O3.write(I0.read()); O3.write(I1.read()); O3.write(I2.read()); O3.write(I3.read());
    O3.write(I4.read()); O3.write(I5.read()); O3.write(I6.read()); O3.write(I7.read());
  }
  void action4() {
    O4.write(I0.read()); O4.write(I1.read()); O4.write(I2.read()); O4.write(I3.read());
    O4.write(I4.read()); O4.write(I5.read()); O4.write(I6.read()); O4.write(I7.read());
  }
  void action5() {
    O5.write(I0.read()); O5.write(I1.read()); O5.write(I2.read()); O5.write(I3.read());
    O5.write(I4.read()); O5.write(I5.read()); O5.write(I6.read()); O5.write(I7.read());
  }
  void action6() {
    O6.write(I0.read()); O6.write(I1.read()); O6.write(I2.read()); O6.write(I3.read());
    O6.write(I4.read()); O6.write(I5.read()); O6.write(I6.read()); O6.write(I7.read());
  }
  void action7() {
    O7.write(I0.read()); O7.write(I1.read()); O7.write(I2.read()); O7.write(I3.read());
    O7.write(I4.read()); O7.write(I5.read()); O7.write(I6.read()); O7.write(I7.read());
  }
  void action() {
    while (true) {
      action0();
      action1();
      action2();
      action3();
      action4();
      action5();
      action6();
      action7();
    }
  }
public:
  SC_HAS_PROCESS(m_transpose);

  m_transpose(sc_module_name name)
    : sc_module(name) {
    SC_THREAD(action);
  }
};
#endif // _INCLUDED_TRANSPOSE_HPP
