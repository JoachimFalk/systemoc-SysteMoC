// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
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

#include <systemoc/smoc_node_types.hpp>

void smoc_actor::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_actor::finalise() begin, name == " << this->name() << std::endl;
#endif
  // Preallocate ID
  //smoc_modes::PGWriter::getId(this);
  
  _currentState = _initialState.finalise(this);

  smoc_root_node::finalise();
  
  //check for non strict transitions
  const smoc_firing_rules               &fsmRules  = _initialState.getFiringRules(); 
  const smoc_firing_types::statelist_ty &fsmStates = fsmRules.getFSMStates(); 
  
  for (smoc_firing_rules::statelist_ty::const_iterator fsmiter =fsmStates.begin(); 
       fsmiter != fsmStates.end(); 
       ++fsmiter) {
    const smoc_firing_types::transitionlist_ty &cTraSt = (*fsmiter)->tl;
    
    for (smoc_firing_types::transitionlist_ty::const_iterator titer = cTraSt.begin(); 
         titer != cTraSt.end(); 
         ++titer ) {
      const smoc_firing_types::statelist_ty &cToNState = titer->sl;
      
      assert( cToNState.size() <= 1 );
      if ( cToNState.size() == 1 ) {
        if (CoSupport::isType<smoc_sr_func_pair>(titer->f)) {
#ifdef SYSTEMOC_DEBUG
          cout << "found non strict SR block: " << this->name() << endl;
#endif
          _non_strict = true;
        }
      }
    }
  }
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_root_node::finalise() end, name == " << this->name() << std::endl;
#endif
}
