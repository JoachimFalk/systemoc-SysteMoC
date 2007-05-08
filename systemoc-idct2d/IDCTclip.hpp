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

#ifndef _INCLUDED_IDCTCLIP_HPP
#define _INCLUDED_IDCTCLIP_HPP

#ifdef VERBOSE_ACTOR
#define VERBOSE_IDCT_CLIP
#endif

class m_IDCTclip: public smoc_actor {
public:
  smoc_port_in<int>  I;
  smoc_port_in<int>  MIN;
  smoc_port_out<int> O;
private:
  const int MAX;
  
  int bound(int a, int x, int b) {
    return x < a 
      ? a
      : ( x > b
          ? b
          : x );
  }
  
  void action0() { 
		int O_internal = bound(MIN[0], I[0], MAX);
#ifdef VERBOSE_IDCT_CLIP
#ifndef NDEBUG
#ifndef XILINX_EDK_RUNTIME
		cout << name() << ": " << "I[0] = " << I[0] << endl;
		cout << name() << ": " << "MIN[0] = " << MIN[0] << endl;
		cout << name() << ": " << "MAX = " << MAX << endl;
		cout << name() << ": " << "O[0] = " << O_internal << endl;
#else
		xil_printf("%s: I[0] = %d\r\n",name(),I[0]);
		xil_printf("%s: MIN[0] = %d\r\n",name(),MIN[0]);
		xil_printf("%s: MAX = %d\r\n",name(),MAX);
		xil_printf("%s: O[0] = %d\r\n",name(),O_internal);
#endif
#endif
#endif
    O[0] = O_internal; 
	}
  
    smoc_firing_state start;
public:
  m_IDCTclip(sc_module_name name,
             int MAX)
    : smoc_actor(name, start),
      MAX(MAX) {
    SMOC_REGISTER_CPARAM(MAX);
    start = (I(1) && MIN(1))          >>
            O(1)                      >>
            CALL(m_IDCTclip::action0) >> start;
  }
  virtual ~m_IDCTclip(){}
};

#endif // _INCLUDED_IDCTCLIP_HPP
