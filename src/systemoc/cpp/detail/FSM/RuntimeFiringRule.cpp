// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include "RuntimeFiringRule.hpp"

#include <smoc/detail/DebugOStream.hpp>

#include <tuple>
#include <cassert>
#include <set>
#include <map>

namespace smoc { namespace Detail { namespace FSM {

  typedef SimulatorAPI::FunctionNames FunctionNames;

  class RuntimeFiringRule::GuardVisitor: public ExprVisitor<void> {
  private:
    typedef ExprVisitor<void> base_type;
    typedef GuardVisitor      this_type;
  public:
    GuardVisitor(RuntimeFiringRule &fr)
      : fr(fr) {}

    result_type visitVar(std::string const &name, std::string const &type) {
      fr.guardComplexity += 2;
      return nullptr;
    }
    result_type visitLiteral(std::string const &type, std::string const &value) {
      fr.guardComplexity += 1;
      return nullptr;
    }
    result_type visitMemGuard(
        std::string const &name, std::string const &cxxType,
        std::string const &reType, ParamInfoList const &params) {
      fr.guardNames.push_back(name);
      return nullptr;
    }
    result_type visitEvent(smoc_event_waiter &e, std::string const &name) {
      fr.events &= e;
      fr.guardComplexity += 1;
      return nullptr;
    }
    result_type visitToken(PortBase &p, size_t n){
      fr.guardComplexity += 5;
      return nullptr;
    }
    result_type visitComm(PortBase &p, size_t c, size_t r) {
      fr.guardComplexity += 3;
      if (p.isInput()) {
        fr.portInInfos.push_back(PortInInfo(static_cast<PortInBase &>(p), c, r));
        assert(c <= r);
      } else {
        fr.portOutInfos.push_back(PortOutInfo(static_cast<PortOutBase &>(p), c, r));
        // Cluster FSM guards have a consume of zero
        assert(c == 0 || c == r);
      }
      return nullptr;
    }
    result_type visitUnOp(OpUnT op,
        std::function<result_type (base_type &)> e){
      fr.guardComplexity += 1;
      e(*this);
      return nullptr;
    }
    result_type visitBinOp(OpBinT op,
        std::function<result_type (base_type &)> a,
        std::function<result_type (base_type &)> b){
      fr.guardComplexity += 1;
      a(*this);
      b(*this);
      return nullptr;
    }
  private:
    RuntimeFiringRule &fr;
  };

  static
  smoc_event_and_list *getCAP(const smoc_event_and_list &ap) {
    typedef std::set<smoc_event_and_list> Cache;
    static Cache* cache = new Cache();
    return &const_cast<smoc_event_and_list &>(*cache->insert(ap).first);
  }

  RuntimeFiringRule::RuntimeFiringRule(smoc_guard const &g, smoc_action const &f)
    : smoc_firing_rule(g,f)
    , guardComplexity(0)
    , ioPatternWaiter(nullptr)
  {
    GuardVisitor visitor(*this);
    Expr::evalTo(visitor, getGuard());
  }

  void RuntimeFiringRule::end_of_elaboration() {
    for (PortInInfo portInfo : getPortInInfos()) {
      events &= static_cast<PortInBase &>(portInfo.port).blockEvent(portInfo.required);
    }
    for (PortOutInfo portInfo : getPortOutInfos()) {
      events &= static_cast<PortOutBase &>(portInfo.port).blockEvent(portInfo.required);
    }
    ioPatternWaiter = getCAP(events);
    events.clear();
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  FunctionNames RuntimeFiringRule::getGuardNames() const {
    return guardNames;
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  size_t        RuntimeFiringRule::getGuardComplexity() const {
    return guardComplexity;
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  FunctionNames RuntimeFiringRule::getActionNames() const {
    FunctionNames actionNames;

    for (smoc_action::value_type const &f : getAction()) {
      actionNames.push_back(f->getFuncName());
    }
    return actionNames;
  }

  RuntimeFiringRule::~RuntimeFiringRule() {
  }

} } } // namespace smoc::Detail::FSM
