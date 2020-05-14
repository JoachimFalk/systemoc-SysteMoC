// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
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
    : srcState(srcStateRegex)
    , action(actionRegex)
    , dstState(dstStateRegex)
    , preCallback(pre)
    , postCallback(post) {}

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
