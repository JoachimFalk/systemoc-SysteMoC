// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_RUNTIMEFIRINGRULE_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_RUNTIMEFIRINGRULE_HPP

#include <smoc/SimulatorAPI/FiringRuleInterface.hpp>
#include <smoc/smoc_firing_rule.hpp>
#include <smoc/detail/VpcInterface.hpp>

#include <smoc/smoc_guard.hpp>
#include <systemoc/detail/smoc_func_call.hpp> // for smoc_action
#include <smoc/smoc_event.hpp>
#include <smoc/detail/PortBase.hpp>

#include <systemoc/smoc_config.h>

#include <vector>

namespace smoc { namespace Detail { namespace FSM {

  class RuntimeFiringRule
    : public SimulatorAPI::FiringRuleInterface
    , public smoc_firing_rule
#ifdef SYSTEMOC_ENABLE_VPC
    , public VpcTaskInterface
#endif // SYSTEMOC_ENABLE_VPC
  {
  public:
    typedef SimulatorAPI::FunctionNames FunctionNames;

    RuntimeFiringRule(smoc_guard const &g, smoc_action const &f);

    /// @brief Returns event waiter for input/output guards (enough token/free space)
    smoc_event_waiter *getIOPatternWaiter() const
      { assert(ioPatternWaiter); return ioPatternWaiter; }

    /// Implement SimulatorAPI::FiringRuleInterface
    void          freeInputs();
    /// Implement SimulatorAPI::FiringRuleInterface
    void          releaseOutputs();
    /// Implement SimulatorAPI::FiringRuleInterface
    FunctionNames getGuardNames() const;
    /// Implement SimulatorAPI::FiringRuleInterface
    size_t        getGuardComplexity() const;
    /// Implement SimulatorAPI::FiringRuleInterface
    FunctionNames getActionNames() const;
  private:
    class GuardVisitor;

    struct PortInfo {
      PortInfo(PortBase &p, size_t c, size_t r)
        : port(p), commited(c), required(r) {}

      PortBase     &port;
      size_t const  commited;
      size_t const  required;
    };

    typedef std::vector<PortInfo> PortInfos;

    PortInfos     portInfos;
    FunctionNames guardNames;
    size_t        guardComplexity;

    /// @brief Event waiter for input/output guards (enough token/free space)
    smoc_event_and_list *ioPatternWaiter;
  };

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_RUNTIMEFIRINGRULE_HPP */
