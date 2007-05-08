// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
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

#ifndef _INCLUDED_IDCTSCALE_HPP
#define _INCLUDED_IDCTSCALE_HPP

#ifdef VERBOSE_ACTOR
#define VERBOSE_IDCT_SCALE
#endif

class m_IDCTscale: public smoc_actor {
public:
  smoc_port_in<int> I;
  smoc_port_out<int> O;
private:
  const int  G;
  const int  OS;
  
  void action0() {
		int temp = OS + (G * I[0]);
#ifdef VERBOSE_IDCT_SCALE
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "I[0] = " << I[0] << endl;
		cout << name() << ": " << "O[0] = " << temp << endl;
#else
		xil_printf("%s: I[0] = %d\r\n",name(),I[0]);
		xil_printf("%s: O[0] = %d\r\n",name(),temp);
#endif
#endif
#endif
    O[0] = temp;
  }
  
  smoc_firing_state start;
public:
  m_IDCTscale(sc_module_name name,
              int G, int OS)
    : smoc_actor(name, start),
      G(G), OS(OS) {
    SMOC_REGISTER_CPARAM(G);
    SMOC_REGISTER_CPARAM(OS);
    start = I(1) >> O(1)                >>
            CALL(m_IDCTscale::action0)  >> start;
  }
};
#endif // _INCLUDED_IDCTSCALE_HPP
