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

#ifndef _INCLUDED_IDCTFLY_HPP
#define _INCLUDED_IDCTFLY_HPP

#include <systemc.h>

class m_IDCTfly: public sc_module {
public:
  sc_fifo_in<int> I1;
  sc_fifo_in<int> I2;
  sc_fifo_out<int> O1;
  sc_fifo_out<int> O2;
private:
  const int  W0;
  const int  OS;
  const int  W1;
  const int  W2;
  const int  ATTEN;
  
  void action0() {
    while (true) {
      int i1 = I1.read();
      int i2 = I2.read();
      int t = (W0 * (i1 + i2)) + OS;
      O1.write(cal_rshift(t + (i1 * W1), ATTEN));
      O2.write(cal_rshift(t + (i2 * W2), ATTEN));
    }
  }
public:
  SC_HAS_PROCESS(m_IDCTfly);
  
  m_IDCTfly(sc_module_name name, int W0, int OS, int W1, int W2, int ATTEN)
    : sc_module(name),
      W0(W0), OS(OS), W1(W1), W2(W2), ATTEN(ATTEN) {
    SC_THREAD(action0);
  }
};

#endif // _INCLUDED_IDCTFLY_HPP
