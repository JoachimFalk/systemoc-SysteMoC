// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_RUNTIMEFIRINGRULE_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_RUNTIMEFIRINGRULE_HPP

#include <smoc/SimulatorAPI/FiringRuleInterface.hpp>
#include <smoc/smoc_firing_rule.hpp>
#include <smoc/smoc_event.hpp>

#include <smoc/smoc_guard.hpp>
#include <smoc/smoc_action.hpp>
#include <smoc/smoc_event.hpp>
#include <smoc/detail/PortBase.hpp>

#include <systemoc/smoc_config.h>

#include <boost/noncopyable.hpp>

#include <vector>
#include <list>

namespace smoc { namespace Detail { namespace FSM {

  class RuntimeFiringRule
    : public SimulatorAPI::FiringRuleInterface
    , public smoc_firing_rule
    , private boost::noncopyable
  {
  public:
    typedef SimulatorAPI::FunctionNames FunctionNames;

    RuntimeFiringRule(smoc_guard const &g, smoc_action const &f);

    /// @brief Returns event waiter for input/output guards (enough token/free space)
    smoc_event_waiter *getIOPatternWaiter() const
      { assert(ioPatternWaiter); return ioPatternWaiter; }

    /// Implement SimulatorAPI::FiringRuleInterface
    FunctionNames getGuardNames() const;
    /// Implement SimulatorAPI::FiringRuleInterface
    size_t        getGuardComplexity() const;
    /// Implement SimulatorAPI::FiringRuleInterface
    FunctionNames getActionNames() const;

    /// @brief Initialize ioPatternWaiter after channels have been bound.
    void end_of_elaboration();

    void          commSetup() {
#if defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)
      for (PortInInfo const &portInfo : getPortInInfos()) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
        static_cast<PortInBase &>(portInfo.port).traceCommSetup(portInfo.required);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
# ifdef SYSTEMOC_ENABLE_DEBUG
        static_cast<PortInBase &>(portInfo.port).setLimit(portInfo.required);
# endif // SYSTEMOC_ENABLE_DEBUG
      }
      for (PortOutInfo const &portInfo : getPortOutInfos()) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
        static_cast<PortOutBase &>(portInfo.port).traceCommSetup(portInfo.produced);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
# ifdef SYSTEMOC_ENABLE_DEBUG
        static_cast<PortOutBase &>(portInfo.port).setLimit(portInfo.produced);
# endif // SYSTEMOC_ENABLE_DEBUG
      }
#endif // defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)
    }

    void          commReset() {
#if defined(SYSTEMOC_ENABLE_DEBUG)
      for (PortInInfo const &portInfo : getPortInInfos())
        static_cast<PortInBase &>(portInfo.port).setLimit(0);
      for (PortOutInfo const &portInfo : getPortOutInfos())
        static_cast<PortOutBase &>(portInfo.port).setLimit(0);
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    }

    ~RuntimeFiringRule();
  private:
    class GuardVisitor;

    FunctionNames guardNames;
    size_t        guardComplexity;

    /// @brief Event waiter for input/output guards (enough token/free space)
    smoc_event_and_list *ioPatternWaiter;
  };

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_RUNTIMEFIRINGRULE_HPP */
