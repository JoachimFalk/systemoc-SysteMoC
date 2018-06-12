// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_HOOKING

#include <smoc/smoc_actor.hpp>

#include "RuntimeTransitionHook.hpp"
#include "FiringFSM.hpp"

namespace smoc { namespace Detail { namespace FSM {

  RuntimeTransitionHook::RuntimeTransitionHook(
      std::string const &srcStateRegex,
      std::string const &actionRegex,
      std::string const &dstStateRegex,
      smoc_pre_hook_callback  const &pre,
      smoc_post_hook_callback const &post)
    : srcState(srcState), action(action), dstState(dstState),
      preCallback(pre), postCallback(post) {}

  bool RuntimeTransitionHook::match(
      std::string const &srcState,
      std::string const &actionStr,
      std::string const &dstState) const
  {
    return boost::regex_search(srcState, this->srcState) &&
           boost::regex_search(actionStr, this->action) &&
           boost::regex_search(dstState, this->dstState);
  }

} } } // namespace smoc::Detail::FSM

namespace smoc {

  void smoc_add_transition_hook(smoc_actor *node,
    std::string const &srcStateRegex,
    std::string const &actionRegex,
    std::string const &dstStateRegex,
    smoc_pre_hook_callback  const &pre,
    smoc_post_hook_callback const &post)
{
  node->getFiringFSM()->addTransitionHook(srcStateRegex, actionRegex, dstStateRegex, pre, post);
}

} // namespace smoc

#endif // SYSTEMOC_ENABLE_HOOKING
