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

template <typename T>
class Select: public smoc_actor {
public:
  smoc_port_in<int>  Control;
  smoc_port_in<T>    Data0, Data1;
  smoc_port_out<T>   Output;
private:
  void action0() { Output[0] = Data0[0] ; }
  void action1() { Output[0] = Data0[1] ; }
  smoc_firing_state start;
public:
  Select(sc_module_name name, int initialChannel = 0)
    : smoc_actor(name, start) {
    smoc_firing_state atChannel0, atChannel1;
    
    atChannel0
      = (Control.getAvailableTokens() >= 1 &
         Data0.getAvailableTokens()   >= 1 &
         Control.getValueAt(0)        == 0   ) >>
        (Output.getAvailableSpace()   >= 1   ) >>
        call(&Select::action0)                 >> atChannel0
      | (Control.getAvailableTokens() >= 1 &
         Data1.getAvailableTokens()   >= 1 &
         Control.getValueAt(0)        == 1   ) >>
        (Output.getAvailableSpace()   >= 1   ) >> 
        call(&Select::action1)                 >> atChannel1
      | (Data0.getAvailableTokens()   >= 1   ) >>
        (Output.getAvailableSpace()   >= 1   ) >>
         call(&Select::action0)                >> atChannel0;

    atChannel1
      = (Control.getAvailableTokens() >= 1 &
         Data1.getAvailableTokens()   >= 1 &
         Control.getValueAt(0)        == 1   ) >>
        (Output.getAvailableSpace()   >= 1   ) >>
        call(&Select::action1)                 >> atChannel1
      | (Control.getAvailableTokens() >= 1 &
         Data0.getAvailableTokens()   >= 1 &
         Control.getValueAt(0)        == 0   ) >>
        (Output.getAvailableSpace()   >= 1   ) >> 
        call(&Select::action0)                 >> atChannel0
      | (Data1.getAvailableTokens()   >= 1   ) >>
        (Output.getAvailableSpace()   >= 1   ) >>
         call(&Select::action1)                >> atChannel1;
    
    start = initialChannel == 0
      ? atChannel0
      : atChannel1;
  }
};
