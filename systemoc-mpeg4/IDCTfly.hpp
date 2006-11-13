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


class m_IDCTfly: public smoc_actor {
public:
  smoc_port_in<int> I1;
  smoc_port_in<int> I2;
  smoc_port_out<int> O1;
  smoc_port_out<int> O2;
private:
  const int  W0;
  const int  OS;
  const int  W1;
  const int  W2;
  const int  ATTEN;
  
  void action0() {
    int t = (W0 * (I1[0] + I2[0])) + OS;
    O1[0] = cal_rshift(t + (I1[0] * W1), ATTEN);
    O2[0] = cal_rshift(t + (I2[0] * W2), ATTEN);

    //std::cout<<name()<<"  M_IDCTfly Debugzeile hier ist I1 O1 wert: "<< I1[0]<<" "<<O1[0] <<"\n";
    //std::cout<<name()<<"  M_IDCTfly Debugzeile hier ist I2 O2 wert: "<< I2[0]<<" "<<O2[0] <<"\n";
  }
  
  smoc_firing_state start;
public:
  m_IDCTfly(sc_module_name name,
            SMOC_ACTOR_CPARAM(int, W0),
	    SMOC_ACTOR_CPARAM(int, OS),
	    SMOC_ACTOR_CPARAM(int, W1),
	    SMOC_ACTOR_CPARAM(int, W2),
	    SMOC_ACTOR_CPARAM(int, ATTEN))
    : smoc_actor(name, start),
      W0(W0), OS(OS), W1(W1), W2(W2), ATTEN(ATTEN) {
    start = (I1(1) && I2(1))          >>
            (O1(1) && O2(1))          >>
            CALL(m_IDCTfly::action0)  >> start;
  }
  
  virtual ~m_IDCTfly() {}
};


#endif // _INCLUDED_IDCTFLY_HPP
