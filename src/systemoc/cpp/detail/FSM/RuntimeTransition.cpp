// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
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

#include <smoc/smoc_actor.hpp>
#include <smoc/SimulatorAPI/SchedulerInterface.hpp>

#include "RuntimeTransition.hpp"
#include "RuntimeState.hpp"
#include "RuntimeFiringRule.hpp"
#include "../SimulationContext.hpp"

#include <systemoc/smoc_config.h>

namespace smoc { namespace Detail { namespace FSM {

  class TransitionActionNameVisitor {
  public:
    typedef std::string result_type;
  public:
    result_type operator()(smoc_action const &f) const {
      std::ostringstream str;

      for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
        if (i != f.begin())
          str << ";";
        str << (*i)->getFuncName() << "(";
        ParamInfoList pil = (*i)->getParams();
        for (ParamInfoList::const_iterator pIter = pil.begin();
             pIter != pil.end();
             ++pIter) {
          if (pIter != pil.begin())
            str << ",";
          str << pIter->value;
        }
        str << ")";
      }
      return str.str();
    }
  };

  /// @brief Constructor
  RuntimeTransition::RuntimeTransition(
#ifdef SYSTEMOC_ENABLE_HOOKING
    RuntimeTransitionHooks const &transitionHooks,
    RuntimeState                 *srcState,
#endif //SYSTEMOC_ENABLE_HOOKING
    RuntimeFiringRule            *firingRule,
    RuntimeState                 *destState
  ) : SimulatorAPI::TransitionInterface(firingRule)
    , destState(destState)
#ifdef SYSTEMOC_ENABLE_HOOKING
    , actionStr(TransitionActionNameVisitor()(getAction()))
#endif //SYSTEMOC_ENABLE_HOOKING
  {
    assert(firingRule);
    assert(destState);
#ifdef SYSTEMOC_NEED_IDS
    // Allocate Id for myself.
    getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_ENABLE_HOOKING
    assert(srcState);
    for (RuntimeTransitionHook const &th : transitionHooks) {
      if (th.match(srcState->name(), actionStr, destState->name())) {
        preHooks.push_back(&th.preCallback);
        postHooks.push_back(&th.postCallback);
      }
    }
#endif //SYSTEMOC_ENABLE_HOOKING
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
    if (getSimCTX()->isTraceDumpingEnabled()) {
      getSimCTX()->getTraceFile() << "<functions transition_id=\"" << getId() << "\">";
      for(std::string const &guardName : getFiringRule()->getGuardNames()) {
        getSimCTX()->getTraceFile() << " " << guardName;
      }
      for(std::string const &actionName : getFiringRule()->getActionNames()) {
        getSimCTX()->getTraceFile() << " " << actionName;
      }
      getSimCTX()->getTraceFile() << "</functions>\n";
    }
#endif //SYSTEMOC_ENABLE_TRANSITION_TRACE
  }

  bool RuntimeTransition::check() const {
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "[" << getIOPatternWaiter() << "] " << *getIOPatternWaiter() << std::endl;
    }
#endif // SYSTEMOC_ENABLE_DEBUG
    bool result = getIOPatternWaiter()->isActive();
    if (result) {
      getFiringRule()->commSetup();
      smoc::Detail::ActivationStatus retval =
          smoc::Expr::evalTo<smoc::Expr::Value>(getGuard());
      getFiringRule()->commReset();
      switch (retval.toSymbol()) {
        case smoc::Detail::_ENABLED:
          break;
        case smoc::Detail::_DISABLED:
          result = false;
          break;
        default:
          assert(!"WHAT?!");
          result = false;
      }
    }
    return result;
  }

  RuntimeState *RuntimeTransition::execute(NodeBase *node) {
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "<transition actor=\"" << node->name()
           << "\">" << std::endl << smoc::Detail::Indent::Up;
    }
#endif //SYSTEMOC_ENABLE_DEBUG

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceStartActor(node, "s");
    this->getSimCTX()->getDataflowTraceLog()->traceTransition(getId());
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE

    getFiringRule()->commSetup();

#ifdef SYSTEMOC_ENABLE_HOOKING
    for (PreHooks::const_iterator iter = preHooks.begin();
         iter != preHooks.end();
         ++iter) {
      (*iter)->operator()(static_cast<smoc_actor *>(node), node->getCurrentState()->name(), actionStr, destState->name());
    }
#endif // SYSTEMOC_ENABLE_HOOKING

    getAction()(); // Call the action

#ifdef SYSTEMOC_ENABLE_HOOKING
    for (PostHooks::const_iterator iter = postHooks.begin();
         iter != postHooks.end();
         ++iter) {
      (*iter)->operator()(static_cast<smoc_actor *>(node), node->getCurrentState()->name(), actionStr, destState->name());
    }
#endif // SYSTEMOC_ENABLE_HOOKING

#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
    if (getSimCTX()->isTraceDumpingEnabled())
      getSimCTX()->getTraceFile() << "<t id=\"" << getId() << "\"/>\n";
#endif // SYSTEMOC_ENABLE_TRANSITION_TRACE

    node->getScheduler()->executeFiringRule(node, getFiringRule());
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    if(!*events.latency) {
      // latency event not signaled
      events.latency->addListener(new smoc::Detail::DeferedTraceLogDumper(node, "l"));
    } else {
      this->getSimCTX()->getDataflowTraceLog()->traceStartActor(node, "l");
      this->getSimCTX()->getDataflowTraceLog()->traceEndActor(node);
    }
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE

    getFiringRule()->commReset();

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceEndActor(node);
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE

#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</transition>"<< std::endl;
    }
#endif // SYSTEMOC_ENABLE_DEBUG
    return getDestState();
  }

} } } // namespace smoc::Detail::FSM
