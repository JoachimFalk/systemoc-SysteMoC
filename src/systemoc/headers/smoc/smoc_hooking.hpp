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

#ifndef _INCLUDED_SMOC_SMOC_HOOKING_HPP
#define _INCLUDED_SMOC_SMOC_HOOKING_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_HOOKING

#include <string>
#include <functional>

namespace smoc {

class smoc_actor;

typedef std::function<void (smoc_actor *, const std::string &, const std::string &, const std::string &)> smoc_pre_hook_callback;
typedef std::function<void (smoc_actor *, const std::string &, const std::string &, const std::string &)> smoc_post_hook_callback;

/// Add transition hook matching srcStateRegex, actionRegex, and dstStateRegex.
/// For runtime transitions matching the hook, the pre and post callbacks are called
/// before and after the action of the transition has been executed, respectively.
void smoc_add_transition_hook(smoc_actor *,
  std::string const &srcStateRegex,
  std::string const &actionRegex,
  std::string const &dstStateRegex,
  smoc_pre_hook_callback  const &pre,
  smoc_post_hook_callback const &post);

} // namespace smoc

#endif // SYSTEMOC_ENABLE_HOOKING

#endif /* _INCLUDED_SMOC_SMOC_HOOKING_HPP */
