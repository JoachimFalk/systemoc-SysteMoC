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

#include <systemoc/smoc_func_call.hpp>
#include <systemoc/detail/smoc_firing_rules_impl.hpp>

ActionVisitor::ActionVisitor(FiringStateImpl* dest, int mode)
  : dest(dest), mode(mode) {}

FiringStateImpl* ActionVisitor::operator()(smoc_func_call& f) const {
  // Function call
#ifdef SYSTEMOC_TRACE
  TraceLog.traceStartFunction(f.getFuncName());
#endif // SYSTEMOC_TRACE
#ifdef SYSTEMOC_DEBUG
  std::cerr << "    <action type=\"smoc_func_call\" func=\""
            << f.getFuncName() << "\">" << std::endl;
#endif // SYSTEMOC_DEBUG
  
  f();

#ifdef SYSTEMOC_DEBUG
  std::cerr << "    </action>" << std::endl;
#endif // SYSTEMOC_DEBUG
#ifdef SYSTEMOC_TRACE
  TraceLog.traceEndFunction(f.getFuncName());
#endif // SYSTEMOC_TRACE
  assert(dest);
  return dest;
}

FiringStateImpl* ActionVisitor::operator()(smoc_func_diverge& f) const {
  // Function call determines next state (Internal use only)
#ifdef SYSTEMOC_DEBUG
  std::cerr << "    <action type=\"smoc_func_diverge\" func=\"???\">"
            << std::endl;
#endif

  FiringStateImpl* ret = f();

#ifdef SYSTEMOC_DEBUG
  std::cerr << "    </action>" << std::endl;
#endif
  assert(!dest);
  return ret;
}

FiringStateImpl* ActionVisitor::operator()(smoc_sr_func_pair& f) const {
  // SR GO & TICK calls
#ifdef SYSTEMOC_TRACE
  TraceLog.traceStartFunction(f.go.getFuncName());
#endif
  if(mode & ExpandedTransition::GO) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "    <action type=\"smoc_sr_func_pair\" go=\""
              << f.go.getFuncName() << "\">" << std::endl;
#endif
    f.go();
  }
  if(mode & ExpandedTransition::TICK) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "    <action type=\"smoc_sr_func_pair\" tick=\""
              << f.tick.getFuncName() << "\">" << std::endl;
#endif
    f.tick();
  }
#ifdef SYSTEMOC_DEBUG
  std::cerr << "    </action>" << std::endl;
#endif
#ifdef SYSTEMOC_TRACE
  TraceLog.traceEndFunction(f.go.getFuncName());
#endif
  assert(dest);
  return dest;
}

FiringStateImpl* ActionVisitor::operator()(smoc_connector_action_pair& f) const {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "    <action type=\"smoc_connector_action_pair\">"
            << std::endl;
#endif

  //FiringStateImpl* aNext =
  //  boost::apply_visitor(ActionVisitor(dest, mode), f.a);
  //assert(aNext == dest);

  f.a();

  //FiringStateImpl* bNext =
  //  boost::apply_visitor(ActionVisitor(dest, mode), f.b);
  //assert(bNext == dest);

  f.b();

#ifdef SYSTEMOC_DEBUG
  std::cerr << "    </action>" << std::endl;
#endif
  assert(dest);
  return dest;
}

FiringStateImpl* ActionVisitor::operator()(boost::blank& f) const {
  // No action
#ifdef SYSTEMOC_DEBUG
  std::cerr << "    <action type=\"none\">" << std::endl
            << "    </action>" << std::endl;
#endif
  assert(dest);
  return dest;
}
