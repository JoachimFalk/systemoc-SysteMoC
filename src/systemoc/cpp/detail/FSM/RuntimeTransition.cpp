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

#include <smoc/smoc_actor.hpp>

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
        str << i->getFuncName() << "(";
        for (ParamInfoList::const_iterator pIter = i->getParams().begin();
             pIter != i->getParams().end();
             ++pIter) {
          if (pIter != i->getParams().begin())
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
#ifdef SYSTEMOC_ENABLE_MAESTRO
    MetaMap::SMoCActor           &parentActor,
#endif //SYSTEMOC_ENABLE_MAESTRO
    RuntimeState                 *destState
  ) : SimulatorAPI::TransitionInterface(firingRule)
#ifdef SYSTEMOC_ENABLE_MAESTRO
    , Transition(pActor)
#endif //SYSTEMOC_ENABLE_MAESTRO
    , destState(destState)
#ifdef SYSTEMOC_ENABLE_HOOKING
    , actionStr(boost::apply_visitor(TransitionActionNameVisitor(), getAction()))
#endif //SYSTEMOC_ENABLE_HOOKING
  {
    assert(firingRule);
    assert(destState);
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
    //FSMTransition
    this->parent = dynamic_cast<Bruckner::Model::Hierarchical*>(this->parentActor);
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
#ifdef SYSTEMOC_ENABLE_HOOKING
    assert(srcState);
    for (RuntimeTransitionHook const &th : transitionHooks) {
      if (th.match(srcState->name(), actionStr, destState->name())) {
        preHooks.push_back(&th.preCallback);
        postHooks.push_back(&th.postCallback);
      }
    }
#endif //SYSTEMOC_ENABLE_HOOKING
  }

#ifdef SYSTEMOC_ENABLE_MAESTRO
  /**
   * Method to be used by a thread to execute this transition's actions
   */
  void RuntimeTransition::executeTransition(NodeBase* node) {
    this->execute(node);
  }

  bool RuntimeTransition::hasWaitAction() {
    return boost::apply_visitor(Action_HasWaitVisitor(), getAction());
  }
#endif //SYSTEMOC_ENABLE_MAESTRO

  bool RuntimeTransition::check() const {
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "[" << getIOPatternWaiter() << "] " << *getIOPatternWaiter() << std::endl;
    }
#endif // SYSTEMOC_DEBUG
    bool result = getIOPatternWaiter()->isActive();
    if (result) {
#if defined(SYSTEMOC_ENABLE_DEBUG)
      smoc::Expr::evalTo<smoc::Expr::CommSetup>(getGuard());
#endif //defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)
      smoc::Detail::ActivationStatus retval =
          smoc::Expr::evalTo<smoc::Expr::Value>(getGuard());
#if defined(SYSTEMOC_ENABLE_DEBUG)
      smoc::Expr::evalTo<smoc::Expr::CommReset>(getGuard());
#endif // defined(SYSTEMOC_ENABLE_DEBUG)
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
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "<transition actor=\"" << node->name()
           << "\">" << std::endl << smoc::Detail::Indent::Up;
    }
#endif //SYSTEMOC_DEBUG

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceStartActor(node, "s");
    this->getSimCTX()->getDataflowTraceLog()->traceTransition(getId());
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE

#if defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)
    smoc::Expr::evalTo<smoc::Expr::CommSetup>(getGuard());
#endif //defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)

#ifdef SYSTEMOC_ENABLE_HOOKING
    for (PreHooks::const_iterator iter = preHooks.begin();
         iter != preHooks.end();
         ++iter) {
      (*iter)->operator()(static_cast<smoc_actor *>(node), node->getCurrentState()->name(), actionStr, destState->name());
    }
#endif // SYSTEMOC_ENABLE_HOOKING

    // FIXME: Set nextState directly to dest as ActionOnThreadVisitor/ActionVisitor can no longer overwrite this!
#ifdef MAESTRO_ENABLE_POLYPHONIC
    // If parallel execution of actors enable, use ActionOnThreadVisitor.
    RuntimeState *nextState =
      boost::apply_visitor(ActionOnThreadVisitor(dest, MM::MMAPI::getInstance()->runtimeManager), getAction());
#else // !MAESTRO_ENABLE_POLYPHONIC
    RuntimeState *nextState =
      boost::apply_visitor(ActionVisitor(getDestState()), getAction());
#endif // !MAESTRO_ENABLE_POLYPHONIC

#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
    if (this->parentActor->logEnabled) {
      Bruckner::Model::FSMTransition* fsmTransition = (Bruckner::Model::FSMTransition*)(this);
      fsmTransition->logMessage("Ex:", 0, 0, true, true, false);
      //Log Transition actions
      for (std::string actionName : this->actionNames) {
        fsmTransition->logMessage(actionName + ",", 0, 0, false, false, false);
      }
      //Log New State
      Bruckner::Model::State* destState = (Bruckner::Model::State*)(dest);
      destState->logMessage("=", 0, 0, false, false, true);
    }
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)

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

#if defined(SYSTEMOC_ENABLE_VPC)
    VpcTaskInterface *vti = getFiringRule();
    vti->getDiiEvent()->reset();
    smoc::Expr::evalTo<smoc::Expr::CommExec>(getGuard(), VpcInterface(vti));
    SystemC_VPC::EventPair events = getFiringRule()->startCompute();
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    if(!*events.latency) {
      // latency event not signaled
      events.latency->addListener(new smoc::Detail::DeferedTraceLogDumper(node, "l"));
    } else {
      this->getSimCTX()->getDataflowTraceLog()->traceStartActor(node, "l");
      this->getSimCTX()->getDataflowTraceLog()->traceEndActor(node);
    }
# endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE
#else // !defined(SYSTEMOC_ENABLE_VPC)
    smoc::Expr::evalTo<smoc::Expr::CommExec>(getGuard());
#endif // !defined(SYSTEMOC_ENABLE_VPC)

#ifdef SYSTEMOC_ENABLE_DEBUG
    smoc::Expr::evalTo<smoc::Expr::CommReset>(getGuard());
#endif // SYSTEMOC_ENABLE_DEBUG

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceEndActor(node);
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE

#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</transition>"<< std::endl;
    }
#endif // SYSTEMOC_DEBUG
    return nextState;
  }

  void RuntimeTransition::before_end_of_elaboration(NodeBase *node) {
#ifdef SYSTEMOC_NEED_IDS
    // Allocate Id for myself.
    getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
    if (getSimCTX()->isTraceDumpingEnabled()) {
      RuntimeFiringRule::FunctionNames guardNames  = getFiringRule()->getGuardNames();
      RuntimeFiringRule::FunctionNames actionNames = getFiringRule()->getActionNames();

      getSimCTX()->getTraceFile() << "<functions transition_id=\"" << getId() << "\">";
      for(RuntimeFiringRule::FunctionNames::const_iterator iter = guardNames.begin();
          iter != guardNames.end();
          ++iter){
        getSimCTX()->getTraceFile() << " " << *iter;
      }
      for(RuntimeFiringRule::FunctionNames::const_iterator iter = actionNames.begin();
          iter != actionNames.end();
          ++iter){
        getSimCTX()->getTraceFile() << " " << *iter;
      }
      getSimCTX()->getTraceFile() << "</functions>\n";
    }
#endif //SYSTEMOC_ENABLE_TRANSITION_TRACE
#ifdef SYSTEMOC_ENABLE_MAESTRO
    //Fill guardNames
    smoc::dMM::MMGuardNameVisitor gVisitor((this->guardNames));
    smoc::Expr::evalTo(gVisitor, getGuard());

    //Fill actionNames
    boost::apply_visitor(smoc::dMM::MMActionNameVisitor((this->actionNames)), getAction());
#endif //SYSTEMOC_ENABLE_MAESTRO
  }

} } } // namespace smoc::Detail::FSM
