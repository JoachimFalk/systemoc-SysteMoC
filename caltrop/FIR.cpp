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

template <typename T> // actor type parameter T
class FIR: public smoc_actor {
public:
  smoc_port_in<T>       input;
  smoc_port_out<T>      output;
private:
  const int             decimation;
  const int             decimationPhase;
  const int             interpolation;
  // taps parameter unmodifiable after actor instantiation
  const std::vector<T>  taps;
  
  T                     zero;
  int                   phaseLength;
  T                     data;     
  int                   mostRecent;
  T                     tapItem;
  T                     dataItem;
  
  void initialize() {
    zero = zero(taps[0]);
    phaseLength = (taps.size() / interpolation);
    if ((taps.size() % interpolation) != 0) {
      phaseLength = phaseLength + 1;
    }
    // create data array
    data = T(phaseLength);
    // initialize data array with zero
    for ( int i = 0; i <= phaseLength -1; i++) {
      data[i] = 0;
    }
    mostRecent = phaseLength;
  }
  
  // start state of the firing rules state machine
  smoc_firing_state start;
  
  // action function for the firing rules state machine
  void action0() {
    // action code here
  }
public:
  FIR(sc_module_name name, // name of actor
      int decimation,
      int decimationPhase,
      int interpolation,
      const std::vector<T> &taps
  ) : smoc_actor( name, start ),
      // make local copy of parameters
      decimation(decimation),
      decimationPhase(decimationPhase),
      interpolation(interpolation),
      taps(taps)
  {
    initialize();
//  action [a] repeat decimation ==> [b] repeat interpolation
    start = (input.getAvailableTokens() >= decimation) >>
            (output.getAvailableSpace() >= interpolation) >>
            call(&FIR::action0) >> start;
  }
};
