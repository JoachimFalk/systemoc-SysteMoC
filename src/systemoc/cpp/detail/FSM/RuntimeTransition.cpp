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

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <Maestro/PolyphoniC/polyphonic_smoc_func_call.h>
#endif //MAESTRO_ENABLE_POLYPHONIC

#ifdef SYSTEMOC_ENABLE_MAESTRO
//////////////TODO: REVIEW THIS SECTION CODE (Visitor's)

using namespace std;

using namespace smoc::Detail;

namespace MetaMap {
  class Transition;
}


namespace smoc { namespace dMM {

class ActionOnThreadVisitor : public smoc::Detail::SimCTXBase {
public:
  typedef smoc::Detail::FSM::RuntimeState *result_type;

public:
  ActionOnThreadVisitor(result_type dest, MetaMap::Transition* transition);

  result_type operator()(const smoc_action& f) const;

private:
  result_type dest;

  MetaMap::Transition* transition;

  void executeTransition(const smoc_action& f) const;

};

class MMActionNameVisitor {
public:
  typedef void result_type;

public:
  MMActionNameVisitor(list<string> & names):
  functionNames(names) {}

  result_type operator()(const smoc_action& f) const {
    for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
      functionNames.push_back(i->getFuncName());
    }
  }

private:
  list<string> &functionNames;
};

class MMGuardNameVisitor: public ExprVisitor<list<string> > {
public:
  typedef ExprVisitor<list<string> >            base_type;
  typedef MMGuardNameVisitor                    this_type;

public:
  MMGuardNameVisitor(list<string> & names) :
    functionNames(names){}

  result_type visitVar(const std::string &name, const std::string &type){
    return nullptr;
  }
  result_type visitLiteral(const std::string &type,
      const std::string &value){
    return nullptr;
  }
  result_type visitMemGuard(
      const std::string &name, const std::string& cxxType,
      const std::string &reType, const ParamInfoList &params){
    functionNames.push_back(name);
    return nullptr;
  }
  result_type visitEvent(const std::string &name){
    return nullptr;
  }
  result_type visitPortTokens(PortBase &p){
    return nullptr;
  }
  result_type visitToken(PortBase &p, size_t n){
    return nullptr;
  }
  result_type visitComm(PortBase &p,
      std::function<result_type (base_type &)> e){
    return nullptr;
  }
  result_type visitUnOp(OpUnT op,
      std::function<result_type (base_type &)> e){
    e(*this);
    return nullptr;
  }
  result_type visitBinOp(OpBinT op,
      std::function<result_type (base_type &)> a,
      std::function<result_type (base_type &)> b){
    a(*this);
    b(*this);

    return nullptr;
  }
private:
  list<string> &functionNames;
};
} } // namespace smoc::Detail

smoc::dMM::TransitionOnThreadVisitor::TransitionOnThreadVisitor(result_type dest, MetaMap::Transition* tr)
  : dest(dest), transition(tr)
{}

smoc::dMM::TransitionOnThreadVisitor::result_type smoc::dMM::TransitionOnThreadVisitor::operator()(const smoc_action& f) const
{
  boost::thread privateThread;

  bool hasWaitTime = false;

  for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
    string name = i->getFuncName();

    if (i->isWaitCall()) {
      hasWaitTime = true;
    }
  }

  if (!hasWaitTime) {
    privateThread = boost::thread(&TransitionOnThreadVisitor::executeTransition, this, f);
    transition->waitThreadDone();
    //privateThread.join();
  } else {
    // Function call
    for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
# ifdef SYSTEMOC_ENABLE_DEBUG
      if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
        smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
          << i->getFuncName() << "\">" << std::endl;
      }
# endif // SYSTEMOC_ENABLE_DEBUG
      (*i)();
# ifdef SYSTEMOC_ENABLE_DEBUG
      if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
        smoc::Detail::outDbg << "</action>" << std::endl;
      }
# endif // SYSTEMOC_ENABLE_DEBUG
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
    }
  }

  return dest;
}

void smoc::dMM::TransitionOnThreadVisitor::executeTransition(const smoc_action& f) const
{
  // Function call
  for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
# ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
        << i->getFuncName() << "\">" << std::endl;
    }
# endif // SYSTEMOC_ENABLE_DEBUG

    (*i)();

# ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "</action>" << std::endl;
    }
# endif // SYSTEMOC_ENABLE_DEBUG
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
  }

  transition->notifyThreadDone();
}

#endif // SYSTEMOC_ENABLE_MAESTRO

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
#ifdef SYSTEMOC_ENABLE_MAESTRO
# ifdef MAESTRO_ENABLE_BRUCKNER
    //FSMTransition
    this->parent = dynamic_cast<Bruckner::Model::Hierarchical*>(this->parentActor);
# endif //MAESTRO_ENABLE_BRUCKNER
    //Fill guardNames
    smoc::dMM::MMGuardNameVisitor gVisitor((this->guardNames));
    smoc::Expr::evalTo(gVisitor, getGuard());

    //Fill actionNames
    boost::apply_visitor(smoc::dMM::MMActionNameVisitor((this->actionNames)), getAction());
#endif //SYSTEMOC_ENABLE_MAESTRO
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

#ifdef MAESTRO_ENABLE_POLYPHONIC
      // If parallel execution of actors enable, use ActionOnThreadVisitor.
      boost::apply_visitor(ActionOnThreadVisitor(dest, MM::MMAPI::getInstance()->runtimeManager), getAction());
#else // !MAESTRO_ENABLE_POLYPHONIC
      getAction()(); // Call the action
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
