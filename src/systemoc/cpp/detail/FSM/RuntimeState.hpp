// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Liyuan Zhang <liyuan.zhang@cs.fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_RUNTIMESTATE_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_RUNTIMESTATE_HPP

#include <smoc/detail/NamedIdedObj.hpp>
#include <smoc/detail/SimCTXBase.hpp>
#include <smoc/detail/NodeBase.hpp>

#include "RuntimeTransition.hpp"

#include <systemoc/smoc_config.h>

#include <string>

namespace smoc { namespace Detail { namespace FSM {

  typedef std::set<smoc::smoc_event_waiter*> EventWaiterSet;

  class RuntimeState
    :
#ifdef SYSTEMOC_NEED_IDS
      public NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
      public SimCTXBase
  {
      typedef RuntimeState this_type;
      friend class RuntimeTransition;
  public:
    RuntimeState(std::string const &name);

    RuntimeTransitionList       &getTransitions();
    RuntimeTransitionList const &getTransitions() const;

    void addTransition(RuntimeTransition const &t);

    /// @brief Initialize am with ioPatternWaiters of outgoing transitions
    /// after channels have been bound.
    void end_of_elaboration();

    EventWaiterSet am;

    const char *name()
      { return stateName.c_str(); }

    ~RuntimeState();
  private:
    std::string           stateName;
    RuntimeTransitionList tl;

#ifdef SYSTEMOC_NEED_IDS
    // To reflect stateName back to NamedIdedObj base class.
    const char *name() const
      { return stateName.c_str(); }
#endif // SYSTEMOC_NEED_IDS
  };

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_RUNTIMESTATE_HPP */
