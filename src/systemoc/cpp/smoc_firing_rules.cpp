// vim: set sw=2 ts=8:
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

#include <algorithm>
#include <map>
#include <set>
#include <list>
#include <stdexcept>
#include <time.h>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/SmartPtr/RefCountObject.hpp>
#include <CoSupport/DataTypes/oneof.hpp>
#include <CoSupport/String/Concat.hpp>
#include <CoSupport/Math/flog2.hpp>
#include <CoSupport/String/convert.hpp>

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_func_call.hpp>
#include <smoc/smoc_expr.hpp>

#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_graph.hpp>
#include <smoc/detail/TraceLog.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <systemoc/detail/smoc_firing_rules_impl.hpp>
#include <smoc/detail/DebugOStream.hpp>

#include "detail/SimulationContext.hpp"

#ifdef SYSTEMOC_ENABLE_HOOKING
# include <boost/regex.hpp> 
#endif // SYSTEMOC_ENABLE_HOOKING

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <Maestro/PolyphoniC/ActionOnThreadVisitor.hpp>
#endif // MAESTRO_ENABLE_POLYPHONIC

using namespace CoSupport::DataTypes;
using namespace smoc::Detail;
using CoSupport::String::Concat;
using CoSupport::String::asStr;

// Prints duration of FiringFSMImpl::finalise() in secs.
//#define FSM_FINALIZE_BENCHMARK

static const char HIERARCHY_SEPARATOR = '.';
static const char PRODSTATE_SEPARATOR = ',';


template<class C> inline bool single(const C& c) {
  if(c.empty()) return false;
  return ++c.begin() == c.end();
}

template<class T>
void markStates(const T& t, Marking& m)  {
  for(typename T::const_iterator tIter = t.begin();
      tIter != t.end(); ++tIter)
  {
    (*tIter)->mark(m);
  }
}

struct ModelingError : public std::runtime_error {
  ModelingError(const char* desc)
    : std::runtime_error(desc)
  {}
};

TransitionBase::TransitionBase(
    Guard const &g,
    const smoc_action& f)
  : guard(g), f(f), ioPattern(NULL) {}

PartialTransition::PartialTransition(
    Guard const &g,
    const smoc_action& f,
    FiringStateBaseImpl* dest)
  : TransitionBase(g, f),
    dest(dest)
{}

FiringStateBaseImpl* PartialTransition::getDestState() const
  { return dest; }


  

ExpandedTransition::ExpandedTransition(
    const HierarchicalStateImpl* src,
    const CondMultiState& in,
    Guard const &g,
    const smoc_action& f,
    const MultiState& dest)
  : TransitionBase(g, f),
    src(src),
    in(in),
    dest(dest)
{}

ExpandedTransition::ExpandedTransition(
    const HierarchicalStateImpl* src,
    const CondMultiState& in,
    Guard const &g,
    const smoc_action& f)
  : TransitionBase(g, f),
    src(src),
    in(in)
{}

ExpandedTransition::ExpandedTransition(
    const HierarchicalStateImpl* src,
    Guard const &g,
    const smoc_action& f)
  : TransitionBase(g, f),
    src(src)
{}

const HierarchicalStateImpl* ExpandedTransition::getSrcState() const
  { return src; }

const CondMultiState& ExpandedTransition::getCondStates() const
  { return in; }

const MultiState& ExpandedTransition::getDestStates() const
  { return dest; }

/// @brief Constructor
RuntimeTransition::RuntimeTransition(
    const boost::shared_ptr<TransitionImpl> &tip
#ifdef SYSTEMOC_ENABLE_MAESTRO
  , MetaMap::SMoCActor& pActor
#endif //SYSTEMOC_ENABLE_MAESTRO
  , RuntimeState *dest)
  :
#ifdef SYSTEMOC_ENABLE_MAESTRO
    Transition(pActor),
#endif //SYSTEMOC_ENABLE_MAESTRO
    transitionImpl(tip),
    dest(dest)
{
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
  //FSMTransition
  this->parent = dynamic_cast<Bruckner::Model::Hierarchical*>(this->parentActor);
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
}

const smoc::Expr::Ex<bool>::type &RuntimeTransition::getExpr() const
  { return transitionImpl->getExpr(); }

RuntimeState* RuntimeTransition::getDestState() const
  { return dest; }

std::string RuntimeTransition::getDestStateName() const
  { return dest->stateName; }

const smoc_action& RuntimeTransition::getAction() const
  { return transitionImpl->getAction(); }

void *RuntimeTransition::getID() const
  { return transitionImpl.get(); }

class ActionNameVisitor {
public:
  typedef std::string result_type;
public:
  result_type operator()(smoc_func_call_list &f) const {
    std::ostringstream str;
    
    for (smoc_func_call_list::iterator i = f.begin(); i != f.end(); ++i) {
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

class Action_HasWaitVisitor {
public:
  typedef bool result_type;
public:
  result_type operator()(const smoc_func_call_list &f) const {
    for (smoc_func_call_list::const_iterator i = f.begin(); i != f.end(); ++i) {
      std::string name = i->getFuncName();
      if ( name.find("simulateTime") != std::string::npos) {
        return true;
      }
    }
    return false;
  }
};

#ifdef SYSTEMOC_ENABLE_MAESTRO
/**
 * Method to be used by a thread to execute this transition's actions
 */
void RuntimeTransition::executeTransition(Node* node) {
  this->execute(node);
}

bool RuntimeTransition::hasWaitAction() {
  return boost::apply_visitor(Action_HasWaitVisitor(), getAction());
}
#endif //SYSTEMOC_ENABLE_MAESTRO

bool RuntimeTransition::check(bool debug) const {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "[" << getIOPatternWaiter() << "] " << *getIOPatternWaiter() << std::endl;
  }
#endif
  bool result = getIOPatternWaiter()->isActive();
  if (result) {
    smoc::Expr::Detail::ActivationStatus retval =
        smoc::Expr::evalTo<smoc::Expr::Value>(getExpr());
  #if defined(SYSTEMOC_ENABLE_DEBUG)
    smoc::Expr::evalTo<smoc::Expr::CommReset>(getExpr());
  #endif
    switch (retval.toSymbol()) {
      case smoc::Expr::Detail::_ENABLED:
        break;
      case smoc::Expr::Detail::_DISABLED:
        result = false;
        break;
      default:
        assert(!"WHAT?!");
        result = false;
    }
  }
#if defined(SYSTEMOC_ENABLE_VPC)
  if (!debug) {
    transitionImpl->vpcTask.check();
  }
#endif //SYSTEMOC_ENABLE_VPC
  return result;
}

void RuntimeTransition::execute(Node *actor) {
  enum {
    MODE_DIISTART,
    MODE_DIIEND,
    MODE_GRAPH
  } execMode;
  
  // Don't use RTTI due to performance reasons!
  if (actor->isActor()) { 
    execMode =
#ifdef SYSTEMOC_ENABLE_VPC
      actor->getCurrentState() != actor->getCommState()
        ? MODE_DIISTART
        : MODE_DIIEND;
#else //!SYSTEMOC_ENABLE_VPC
      MODE_DIISTART;
#endif //!SYSTEMOC_ENABLE_VPC
  } else { // !actor->isActor()
#ifdef SYSTEMOC_ENABLE_VPC
    assert(actor->getCurrentState() != actor->getCommState());
#endif //SYSTEMOC_ENABLE_VPC
    execMode = MODE_GRAPH;
  }
  
#ifdef SYSTEMOC_DEBUG
  static const char *execModeName[] = { "diiStart", "diiEnd", "graph" };

  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << "<transition actor=\"" << actor->name()
         << "\" mode=\"" << execModeName[execMode]
         << "\">" << std::endl << smoc::Detail::Indent::Up;
  }
#endif //SYSTEMOC_DEBUG
  
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  if (execMode != MODE_GRAPH)
    this->getSimCTX()->getDataflowTraceLog()->traceStartActor(actor, execMode == MODE_DIISTART ? "s" : "e");
  if (execMode == MODE_DIISTART) {
    this->getSimCTX()->getDataflowTraceLog()->traceTransition(getId());
  }
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE
 
#if defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)
  smoc::Expr::evalTo<smoc::Expr::CommSetup>(getExpr());
#endif //defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_ENABLE_DATAFLOW_TRACE)

#ifdef SYSTEMOC_ENABLE_HOOKING
  if (execMode == MODE_DIISTART) {
    if (!hookingValid) {
      actionStr = boost::apply_visitor(ActionNameVisitor(), f);
      
      for (std::list<smoc::Hook::Detail::TransitionHook>::const_iterator iter = actor->transitionHooks.begin();
           iter != actor->transitionHooks.end();
           ++iter) {
        if (boost::regex_search(actor->getCurrentState()->name(), iter->srcState) &&
            boost::regex_search(actionStr, iter->action) &&
            boost::regex_search( dest->name(), iter->dstState)) {
          preHooks.push_back(&iter->preCallback);
          postHooks.push_back(&iter->postCallback);
        }
      }
      hookingValid = true;
    }
    for (PreHooks::const_iterator iter = preHooks.begin();
         iter != preHooks.end();
         ++iter) {
      (*iter)->operator()(static_cast<smoc_actor *>(actor), actor->getCurrentState()->name(), actionStr, dest->name());
    }
//  std::cerr << actor->name() << ": " << actor->getCurrentState()->name() << " => " << dest->name() << std::endl;
  }
#endif // SYSTEMOC_ENABLE_HOOKING
  
  // FIXME: Set nextState directly to dest as ActionOnThreadVisitor/ActionVisitor can no longer overwrite this!
#ifdef MAESTRO_ENABLE_POLYPHONIC
  // If parallel execution of actors enable, use ActionOnThreadVisitor.
  RuntimeState *nextState =
    boost::apply_visitor(ActionOnThreadVisitor(dest, MM::MMAPI::getInstance()->runtimeManager), getAction());
#else
  RuntimeState *nextState =
    boost::apply_visitor(ActionVisitor(dest), getAction());
#endif

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
  if (execMode == MODE_DIISTART) {
    for (PostHooks::const_iterator iter = postHooks.begin();
         iter != postHooks.end();
         ++iter) {
      (*iter)->operator()(static_cast<smoc_actor *>(actor), actor->getCurrentState()->name(), actionStr, dest->name());
    }
  }
#endif // SYSTEMOC_ENABLE_HOOKING

#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
  if (execMode == MODE_DIISTART && getSimCTX()->isTraceDumpingEnabled())
    getSimCTX()->getTraceFile() << "<t id=\"" << getId() << "\"/>\n";
#endif // SYSTEMOC_ENABLE_TRANSITION_TRACE
  
#if defined(SYSTEMOC_ENABLE_VPC)
  if (execMode == MODE_DIISTART) {
    VpcTaskInterface *vti = this->transitionImpl.get();
    vti->getDiiEvent()->reset();
    smoc::Expr::evalTo<smoc::Expr::CommExec>(getExpr(), VpcInterface(vti));
    SystemC_VPC::EventPair events = this->transitionImpl->startCompute();
    
    // Insert magic commstate by saving nextState in the sole outgoing
    // transition of the commState
    actor->getCommState()->getTransitions().front().dest = nextState;
    // and overriding nextState with commState.
    nextState = actor->getCommState();
    
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    if(!*events.latency) {
      // latency event not signaled
      events.latency->addListener(new smoc::Detail::DeferedTraceLogDumper(actor, "l"));
    } else {
      this->getSimCTX()->getDataflowTraceLog()->traceStartActor(actor, "l");
      this->getSimCTX()->getDataflowTraceLog()->traceEndActor(actor);
    }
# endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE
  } else {
    smoc::Expr::evalTo<smoc::Expr::CommExec>(getExpr(), VpcInterface());
  }
#else // !defined(SYSTEMOC_ENABLE_VPC)
  smoc::Expr::evalTo<smoc::Expr::CommExec>(getExpr());
#endif // !defined(SYSTEMOC_ENABLE_VPC)

#ifdef SYSTEMOC_ENABLE_DEBUG
  smoc::Expr::evalTo<smoc::Expr::CommReset>(getExpr());
#endif

#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
  if(execMode != MODE_GRAPH)
    this->getSimCTX()->getDataflowTraceLog()->traceEndActor(actor);
#endif //SYSTEMOC_ENABLE_DATAFLOW_TRACE

  actor->setCurrentState(nextState);
  ///todo:delete r
  //cout << "NextState: " << nextState->name() << " for actor: "<< actor->name() << endl;

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</transition>"<< std::endl;
  }
#endif
}

void RuntimeTransition::before_end_of_elaboration(Node *node) {
#ifdef SYSTEMOC_NEED_IDS
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS
#ifdef SYSTEMOC_ENABLE_VPC
  smoc_actor * actor = dynamic_cast<smoc_actor *>(node);
  if (actor != nullptr) {
    FunctionNames guardNames;
    FunctionNames actionNames;

    smoc::Detail::GuardNameVisitor visitor(guardNames);
    smoc::Expr::evalTo(visitor, getExpr());

    boost::apply_visitor(
        smoc::Detail::ActionNameVisitor(actionNames), getAction());

    //calculate delay for guard
    //initialize VpcTaskInterface
    this->transitionImpl->diiEvent = node->diiEvent;
    this->transitionImpl->vpcTask =
      SystemC_VPC::Director::getInstance().registerActor(actor,
                  node->name(), actionNames, guardNames, visitor.getComplexity());
# ifdef SYSTEMOC_DEBUG_VPC_IF
    this->transitionImpl->actor = node->name();
# endif // SYSTEMOC_DEBUG_VPC_IF
#ifdef SYSTEMOC_ENABLE_TRANSITION_TRACE
    if (getSimCTX()->isTraceDumpingEnabled()){
      getSimCTX()->getTraceFile() << "<functions transition_id=\"" << getId() << "\">";
      for(FunctionNames::const_iterator iter = guardNames.begin();
          iter != guardNames.end();
          ++iter){
        getSimCTX()->getTraceFile() << " " << *iter;
      }
      for(FunctionNames::const_iterator iter = actionNames.begin();
          iter != actionNames.end();
          ++iter){
        getSimCTX()->getTraceFile() << " " << *iter;
      }
      getSimCTX()->getTraceFile() << "</functions>\n";
    }
#endif //SYSTEMOC_ENABLE_TRANSITION_TRACE
  }
#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_ENABLE_MAESTRO
  //Fill guardNames
  smoc::dMM::MMGuardNameVisitor gVisitor((this->guardNames));
  smoc::Expr::evalTo(gVisitor, getExpr());

  //Fill actionNames
  boost::apply_visitor(smoc::dMM::MMActionNameVisitor((this->actionNames)), getAction());
#endif //SYSTEMOC_ENABLE_MAESTRO
}

static
IOPattern *getCachedIOPattern(const IOPattern &iop) {
  typedef std::set<IOPattern> Cache;
  static Cache* cache = new Cache();
  return &const_cast<IOPattern&>(*cache->insert(iop).first);
}

void RuntimeTransition::end_of_elaboration() {
  IOPattern tmp;
  smoc::Expr::evalTo<smoc::Expr::Sensitivity>(getExpr(), tmp);
  tmp.finalise();
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Low)) {
    smoc::Detail::outDbg << "=> " << tmp << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
  IOPattern *iop = getCachedIOPattern(tmp);
  transitionImpl->setIOPattern(iop);
}
 
static int UnnamedStateCount = 0;

RuntimeState::RuntimeState(const std::string name)
  : stateName(name.empty() ? Concat("smoc_firing_state_")(UnnamedStateCount++) : name)
{
  finalise();
}

#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
RuntimeState::RuntimeState(const std::string name, Bruckner::Model::Hierarchical* sParent)
  : State(name),
    stateName(name.empty() ? Concat("smoc_firing_state_")(UnnamedStateCount++) : name)
{
  dynamic_cast<Bruckner::Model::Hierarchical*>(this)->parent = sParent;
  finalise();
}
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)

RuntimeState::~RuntimeState() {
//idPool.unregObj(this);
}

void RuntimeState::finalise() {
#ifdef SYSTEMOC_NEED_IDS  
  // Allocate Id for myself.
  getSimCTX()->getIdPool().addIdedObj(this);
#endif // SYSTEMOC_NEED_IDS  
}

const RuntimeTransitionList& RuntimeState::getTransitions() const
  { return tl; }

RuntimeTransitionList& RuntimeState::getTransitions()
  { return tl; }
  
void RuntimeState::addTransition(const RuntimeTransition& t,
                                 Node *node) {
  tl.push_back(t);
  tl.back().before_end_of_elaboration(node); // FIXME: Fix this hack!
}

void RuntimeState::end_of_elaboration() {
  RuntimeTransitionList::iterator iterEnd = getTransitions().end();
  for (RuntimeTransitionList::iterator iter = getTransitions().begin();
       iter != iterEnd;
       ++iter) {
    iter->end_of_elaboration();
    am.insert(iter->getIOPatternWaiter());
  }
}

FiringFSMImpl::FiringFSMImpl()
  : use_count_(0),
    //actor(0),
    top(0)
{
//  std::cerr << "FiringFSMImpl::FiringFSMImpl() this == " << this << std::endl;
}

FiringFSMImpl::~FiringFSMImpl() {
//  std::cerr << "FiringFSMImpl::~FiringFSMImpl() this == " << this << std::endl;
  assert(use_count_ == 0);  

  delete top;

  for(FiringStateBaseImplSet::const_iterator s = states.begin();
      s != states.end(); ++s)
  {
    assert((*s)->getFiringFSM() == this);
    delete *s;
  }

  for(RuntimeStateSet::const_iterator s = rts.begin();
      s != rts.end(); ++s)
  {
    delete *s;
  }
}

const RuntimeStateSet& FiringFSMImpl::getStates() const
  { return rts; }

RuntimeState* FiringFSMImpl::getInitialState() const
  { return init; }

std::ostream& operator<<(std::ostream& os, const ProdState& p) {
  //os << "(";
  for(ProdState::const_iterator s = p.begin();
      s != p.end(); ++s)
  {
    if (s != p.begin())
      os << PRODSTATE_SEPARATOR;
    assert(!(*s)->getName().empty());
    os << (*s)->getHierarchicalName();
  }
  //os << ")";
  return os;
}

/*
 * isImplied(s,p) == true <=> exists p_i in p: isAncestor(s, p_i) == true
 */
bool isImplied(const HierarchicalStateImpl* s, const ProdState& p) {
  for(ProdState::const_iterator pIter = p.begin();
      pIter != p.end(); ++pIter)
  {
    if(s->isAncestor(*pIter))
      return true;
  }
  return false;
}

bool isImplied(const CondMultiState& c, const ProdState& p) {
  for(CondMultiState::const_iterator cIter = c.begin();
      cIter != c.end(); ++cIter)
  {
    if(isImplied(cIter->first, p)) {
      if(cIter->second) return false;
    }
    else {
      if(!cIter->second) return false;
    }
  }
  return true;
}

void FiringFSMImpl::before_end_of_elaboration(
    Node        *actorOrGraphNode,
    HierarchicalStateImpl *hsinit)
{
//  smoc::Detail::outDbg << "FiringFSMImpl::finalise(...) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

  assert(actorOrGraphNode);
  //actorOrGraphNode = _actor;

//  smoc::Detail::outDbg << "Actor or Graph: " << actorOrGraphNode->name() << std::endl;

#ifdef FSM_FINALIZE_BENCHMARK  
  uint64_t finStart;
  size_t nRunStates = 0;
  size_t nRunTrans = 0;
#endif // FSM_FINALIZE_BENCHMARK

  try {

    // create top state (do not try this in the constructor...)
    top = new XORStateImpl();
    unify(top->getFiringFSM());
    delState(top);

#ifdef FSM_FINALIZE_BENCHMARK
    size_t nLeaf = 0;
    size_t nXor = 0;
    size_t nAnd = 0;
    size_t nTrans = 0;
    for(FiringStateBaseImplSet::iterator sIter = states.begin();
        sIter != states.end(); ++sIter) {
      (*sIter)->countStates(nLeaf, nAnd, nXor, nTrans);
    }
    std::cerr << "#leaf: " << nLeaf << "; #and: " << nAnd << "; #xor: "
             << nXor << "; #transitions: " << nTrans << std::endl;
#endif // FSM_FINALIZE_BENCHMARK


#ifdef FSM_FINALIZE_BENCHMARK  
   struct timespec ts;
   clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
   finStart = ts.tv_sec*1000000000ull + ts.tv_nsec;
#endif // FSM_FINALIZE_BENCHMARK

    // move remaining hierarchical states into top state
//    {smoc::Detail::outDbg << "moving HS to top state" << std::endl;
//    ScopedIndent s1(smoc::Detail::outDbg);
    
    FiringStateBaseImplSet::iterator sIter, sNext;

    bool initStateFound = false;

    for(sIter = states.begin();
        sIter != states.end(); sIter = sNext) {
      ++(sNext = sIter);
      if (HierarchicalStateImpl* hs =
          dynamic_cast<HierarchicalStateImpl*>(*sIter))
      {
        top->add(hs, hsinit == hs); // hs->isAncestor(hsinit));
        if (hsinit == hs)
          initStateFound = true;
      }
    }

    if (!initStateFound)
      throw ModelingError("smoc_firing_fsm: Initial state must be on top hierarchy level");

 //   }
    
    ExpandedTransitionList etl;

//  // finalise states -> expanded transitions
//  {smoc::Detail::outDbg << "finalising states" << std::endl;
//  ScopedIndent s1(smoc::Detail::outDbg);
    
    // finalize all non-hierarchical states, i.e., dynamic_cast<HierarchicalStateImpl*>(...) == nullptr
    for(FiringStateBaseImplSet::iterator sIter = states.begin();
        sIter != states.end(); ++sIter)
    {
      (*sIter)->finalise(etl);
    }

    //top->setCodeAndBits(0,1);
    top->finalise(etl);

//    }

//  // calculate runtime states and transitions
//  {smoc::Detail::outDbg << "calculating runtime states / transitions" << std::endl;
//  ScopedIndent s1(smoc::Detail::outDbg);

    assert(rts.empty());

    typedef std::map<ProdState,RuntimeState*> StateTable;
    typedef StateTable::value_type STEntry;
    StateTable st;

    typedef std::list<StateTable::const_iterator> NewStates;
    NewStates ns;

    // determine initial state
    ProdState psinit;
    top->getInitialState(psinit, Marking());

#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
    init = *rts.insert(new RuntimeState (Concat("")(psinit), dynamic_cast<Bruckner::Model::Hierarchical*>(actorOrGraphNode) )).first;
#else //!defined(SYSTEMOC_ENABLE_MAESTRO) || !defined(MAESTRO_ENABLE_BRUCKNER)
    init = *rts.insert(new RuntimeState(Concat(actorOrGraphNode->name())(":")(psinit))).first;
#endif //!defined(SYSTEMOC_ENABLE_MAESTRO) || !defined(MAESTRO_ENABLE_BRUCKNER)

    ns.push_back(
        st.insert(STEntry(psinit, init)).first);
#ifdef FSM_FINALIZE_BENCHMARK
    nRunStates++;
#endif // FSM_FINALIZE_BENCHMARK

    while(!ns.empty()) {
      const ProdState& s = ns.front()->first;
      RuntimeState* rs = ns.front()->second;

      ns.pop_front();

      //Marking mk;
      //markStates(s, mk);

      for(ExpandedTransitionList::const_iterator t = etl.begin();
          t != etl.end(); ++t)
      {
        if(isImplied(t->getSrcState(), s) &&
           isImplied(t->getCondStates(), s))
        //if(t->getSrcState()->isMarked(mk) &&
        //(areMarked(t->getCondStates(), mk))
        {
          const HierarchicalStateImpl* x =
            t->getSrcState()->getTopState(t->getDestStates(), true);

          Marking mknew;


          // mark user-defined states
          markStates(t->getDestStates(), mknew);

          // get the target prod. state
          ProdState d;
          x->getInitialState(d, mknew);
          
          // mark remaining states in prod. state
          for(ProdState::const_iterator i = s.begin();
              i != s.end(); ++i)
          {
            if(!x->isAncestor(*i)) {
              sassert(d.insert(*i).second);
            }// (*i)->mark(mknew);
          }

          std::pair<StateTable::iterator,bool> ins =
            st.insert(STEntry(d, nullptr));
                
          if (ins.second) {
            // FIXME: construct state name and pass to RuntimeState
            ProdState f = ins.first->first;
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
            ins.first->second = *rts.insert(new RuntimeState(Concat("")(f), dynamic_cast<Bruckner::Model::Hierarchical*>(actorOrGraphNode)	)).first;
#else //!defined(SYSTEMOC_ENABLE_MAESTRO) || !defined(MAESTRO_ENABLE_BRUCKNER)
            ins.first->second = *rts.insert(new RuntimeState(Concat(actorOrGraphNode->name())(":")(f))).first;
#endif //!defined(SYSTEMOC_ENABLE_MAESTRO) || !defined(MAESTRO_ENABLE_BRUCKNER)
            ns.push_back(ins.first);
#ifdef FSM_FINALIZE_BENCHMARK
            nRunStates++;
#endif // FSM_FINALIZE_BENCHMARK
          }

          RuntimeState* rd = ins.first->second;
          assert(rd);

#ifdef SYSTEMOC_ENABLE_MAESTRO
          MetaMap::SMoCActor* a = nullptr;
          if (actorOrGraphNode->isActor()) {
            a = dynamic_cast<MetaMap::SMoCActor*>(actorOrGraphNode);
          }
#endif //SYSTEMOC_ENABLE_MAESTRO
          rs->addTransition(
            RuntimeTransition(
              t->getCachedTransitionImpl(),
#ifdef SYSTEMOC_ENABLE_MAESTRO
              *a,
#endif //SYSTEMOC_ENABLE_MAESTRO
              rd),
            actorOrGraphNode);
#ifdef FSM_FINALIZE_BENCHMARK
          nRunTrans++;
#endif // FSM_FINALIZE_BENCHMARK
        }
      }
    }
  }
  catch(const ModelingError& e) {
    std::cerr << "A modeling error occurred:" << std::endl
              << "  \"" << e.what() << "\"" << std::endl
              << "Please check the model and try again!" << std::endl;
    // give up
    exit(1);
  }

#ifdef FSM_FINALIZE_BENCHMARK  
  struct timespec ts;
  clock_gettime(CLOCK_PROCESS_CPUTIME_ID, &ts);
  uint64_t finEnd = ts.tv_sec*1000000000ull + ts.tv_nsec;

  std::cerr << "Finalised FSM of actor/graph '" << actorOrGraphNode->name() << "' in "
            << ((finEnd - finStart) / 1e9) << "s"
            << "; #states: " << nRunStates << "; #trans: " << nRunTrans
            << std::endl;
#endif // FSM_FINALIZE_BENCHMARK
}

void FiringFSMImpl::end_of_elaboration(
    Node        *actorOrGraphNode)
{
  RuntimeStateSet::iterator iterEnd = getStates().end();
  for (RuntimeStateSet::iterator iter = getStates().begin(); iter != iterEnd; ++iter) {
    (*iter)->end_of_elaboration();
  }
}

void FiringFSMImpl::addState(FiringStateBaseImpl *state) {
  assert(state->getFiringFSM() == this);
  sassert(states.insert(state).second);
}

void FiringFSMImpl::delState(FiringStateBaseImpl *state) {
  assert(state->getFiringFSM() == this);
  sassert(states.erase(state) == 1);
}

void FiringFSMImpl::addRef() {
//  std::cerr << "FiringFSMImpl::add_ref() this == " << this
//            << "; use_count: " << use_count_ << std::endl;
  ++use_count_;
}

bool FiringFSMImpl::delRef() {
//  std::cerr << "FiringFSMImpl::delRef() this == " << this
//            << "; use_count: " << use_count_ << std::endl;
  assert(use_count_); --use_count_;
  return use_count_ == 0;
}

void FiringFSMImpl::unify(this_type *fr) {
  if(this != fr) {
//    std::cerr << "FiringFSMImpl::unify() this == " << this
//              << "; other: " << fr << std::endl;
    
    // patch fsm of all states owned by fr
    for(FiringStateBaseImplSet::iterator sIter = fr->states.begin();
        sIter != fr->states.end(); ++sIter)
    {
      assert((*sIter)->getFiringFSM() == fr);
      (*sIter)->setFiringFSM(this);
      sassert(states.insert(*sIter).second);
    }
    
    //std::cerr << " own use_count: " << use_count_
    //          << "; other use_count: " << fr->use_count_
    //          << "; # merged states: " << states.size()
    //          << std::endl;

    use_count_ += fr->use_count_;
    fr->use_count_ = 0;
    fr->states.clear();    
    
    delete fr;
  }
}

FiringStateBaseImpl::FiringStateBaseImpl()
  : fsm(new FiringFSMImpl()) {
//  std::cerr << "FiringStateBaseImpl::FiringStateBaseImpl() this == "
//            << this << std::endl;
  fsm->addState(this);
}

FiringStateBaseImpl::~FiringStateBaseImpl() {
//  std::cerr << "FiringStateBaseImpl::~FiringStateBaseImpl() this == "
//            << this << std::endl;
}

FiringFSMImpl *FiringStateBaseImpl::getFiringFSM() const
  { return fsm; }

void FiringStateBaseImpl::setFiringFSM(FiringFSMImpl *f)
  { fsm = f; }

//const PartialTransitionList& FiringStateBaseImpl::getPTL() const
//  { return ptl; }

void FiringStateBaseImpl::addTransition(const smoc_transition_list& stl) {
  for(smoc_transition_list::const_iterator st = stl.begin();
      st != stl.end(); ++st)
  {
    addTransition(
        PartialTransition(
          st->getExpr(),
          st->getInterfaceAction().getAction(),
          st->getInterfaceAction().getDestState()->getImpl()));
  }
}
  
void FiringStateBaseImpl::addTransition(const PartialTransitionList& ptl) {
  for(PartialTransitionList::const_iterator pt = ptl.begin();
      pt != ptl.end(); ++pt)
  {
    addTransition(*pt);
  }
}

void FiringStateBaseImpl::addTransition(const PartialTransition& pt) {
  ptl.push_back(pt);

  FiringStateBaseImpl* s = pt.getDestState();
  if(s) fsm->unify(s->getFiringFSM());
}

void FiringStateBaseImpl::clearTransition()
  { ptl.clear(); }

#ifdef FSM_FINALIZE_BENCHMARK
void FiringStateBaseImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXOR, size_t& nTrans) const {
  nTrans += ptl.size();
}
#endif // FSM_FINALIZE_BENCHMARK

void intrusive_ptr_add_ref(FiringStateBaseImpl *p)
  { p->getFiringFSM()->addRef(); }

void intrusive_ptr_release(FiringStateBaseImpl *p)
  { if(p->getFiringFSM()->delRef()) delete p->getFiringFSM(); }





HierarchicalStateImpl::HierarchicalStateImpl(const std::string& name)
  : FiringStateBaseImpl(),
    name(name.empty() ? Concat("smoc_firing_state_")(UnnamedStateCount++) : name),
    parent(0),
    code(0),
    bits(1)
{
  if(name.find(HIERARCHY_SEPARATOR) != std::string::npos)
    assert(!"smoc_hierarchical_state: Invalid state name");
  if(name.find(PRODSTATE_SEPARATOR) != std::string::npos)
    assert(!"smoc_hierarchical_state: Invalid state name");
}

HierarchicalStateImpl::~HierarchicalStateImpl() {
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    assert((*s)->getFiringFSM() == fsm);
    delete *s;
  }
}

void HierarchicalStateImpl::add(HierarchicalStateImpl* state) {
  fsm->unify(state->getFiringFSM());
  fsm->delState(state);
  c.push_back(state);
  state->setParent(this);
}

void HierarchicalStateImpl::setFiringFSM(FiringFSMImpl *fsm) {
  FiringStateBaseImpl::setFiringFSM(fsm);
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    (*s)->setFiringFSM(fsm);
  }
}

const std::string& HierarchicalStateImpl::getName() const
  { return name; }

std::string HierarchicalStateImpl::getHierarchicalName() const {
  if(!parent || parent == fsm->top) {
    return name;
  }
  assert(!name.empty());
  return
    parent->getHierarchicalName() +
    HIERARCHY_SEPARATOR +
    name;
}
  
#ifdef FSM_FINALIZE_BENCHMARK
void HierarchicalStateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXor, size_t& nTrans) const {
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    (*s)->countStates(nLeaf, nAnd, nXor, nTrans);
  }
  FiringStateBaseImpl::countStates(nLeaf, nAnd, nXor, nTrans);
}
#endif // FSM_FINALIZE_BENCHMARK

HierarchicalStateImpl* HierarchicalStateImpl::select(
    const std::string& name)
{
  size_t pos = name.find(HIERARCHY_SEPARATOR);
  std::string top = name.substr(0, pos);

  if(top.empty()) {
    if(pos == std::string::npos)
      return this;
    else
      assert(!"smoc_hierarchical_state: Invalid hierarchical name");
  }
 
  for(C::iterator s = c.begin(); s != c.end(); ++s) {
    if((*s)->getName() == top) {
      if(pos == std::string::npos)
        return (*s);
      else
        return (*s)->select(name.substr(pos + 1));
    }
  }

  assert(!"smoc_hierarchical_state:: Invalid hierarchical name");
}


void HierarchicalStateImpl::setParent(HierarchicalStateImpl* v) {
  assert(v);
  if(parent && v != parent) {
    assert(!"smoc_hierarchical_state: Parent already set");
  }
  parent = v;
}

HierarchicalStateImpl* HierarchicalStateImpl::getParent() const {
  return parent;
}

bool HierarchicalStateImpl::isAncestor(const HierarchicalStateImpl* s) const {
  assert(s);

  if(s == this)
    return true;


  if(s->bits > bits)
    return (code == (s->code >> (s->bits - bits)));
  
  return false;
}
  
void HierarchicalStateImpl::mark(Marking& m) const {
  bool& mm = m[this];
  if(!mm) {
    mm = true;
    if(parent) parent->mark(m);
  }
}

bool HierarchicalStateImpl::isMarked(const Marking& m) const {
  Marking::const_iterator iter = m.find(this);
  return (iter == m.end()) ? false : iter->second;
}

void HierarchicalStateImpl::finalise(ExpandedTransitionList& etl) {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<HierarchicalStateImpl::finalise name=\"" << getName() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "Code: " << code << "; Bits: " << bits << std::endl;
  }
#endif // SYSTEMOC_DEBUG

  if(!c.empty()) {
    size_t cs = c.size();
    size_t cb = CoSupport::Math::flog2c(static_cast<uint32_t>(cs));
    
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << "#C: " << cs << " -> CB: " << cb << std::endl;
    }
#endif // SYSTEMOC_DEBUG

    uint64_t cc = code << cb;
    
    cb += bits;
    assert(cb < 64);

    for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
      (*s)->code = cc;
      (*s)->bits = cb;
      (*s)->finalise(etl);
      ++cc;
    }
  }

  for(PartialTransitionList::const_iterator pt = ptl.begin();
      pt != ptl.end(); ++pt)
  {
    assert(pt->getDestState());
    pt->getDestState()->expandTransition(
        etl,
        ExpandedTransition(
          this,
          pt->getExpr(),
          pt->getAction()));
  }

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</HierarchicalStateImpl::finalise>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void HierarchicalStateImpl::expandTransition(
    ExpandedTransitionList& etl,
    const ExpandedTransition& t) const
{
//  smoc::Detail::outDbg << "HierarchicalStateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);
  
  assert(t.getDestStates().empty());

  MultiState dest;
  dest.insert(this);

  etl.push_back(
      ExpandedTransition(
        t.getSrcState(),
        t.getCondStates(),
        t.getExpr(),
        t.getAction(),
        dest));
}

void intrusive_ptr_add_ref(HierarchicalStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(HierarchicalStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }



FiringStateImpl::FiringStateImpl(const std::string& name)
  : HierarchicalStateImpl(name)
{}

void FiringStateImpl::getInitialState(
    ProdState& p, const Marking& m) const
{
//  smoc::Detail::outDbg << "FiringStateImpl::getInitialState(p,m) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

  p.insert(this);
}

#ifdef FSM_FINALIZE_BENCHMARK
void FiringStateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXor, size_t& nTrans) const {
  nLeaf++;
  HierarchicalStateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
}
#endif // FSM_FINALIZE_BENCHMARK

const HierarchicalStateImpl* FiringStateImpl::getTopState(
    const MultiState& d,
    bool isSrcState) const
{
  for(MultiState::const_iterator s = d.begin();
      s != d.end(); ++s)
  {
    if(*s != this) {
      assert(getParent());
      return getParent()->getTopState(d, false);
    }
  }

  return this;
}

void intrusive_ptr_add_ref(FiringStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(FiringStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }



XORStateImpl::XORStateImpl(const std::string& name)
  : HierarchicalStateImpl(name),
    init(0)
{}

void XORStateImpl::add(HierarchicalStateImpl* state, bool i) {
  HierarchicalStateImpl::add(state); 
  if(i) init = state;
}

void XORStateImpl::finalise(ExpandedTransitionList& etl) {
  if(!init)
    throw ModelingError("smoc_xor_state: Must specify initial state");
  HierarchicalStateImpl::finalise(etl);
}

#ifdef FSM_FINALIZE_BENCHMARK
void XORStateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXor, size_t& nTrans) const {
  nXor++;
  HierarchicalStateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
}
#endif // FSM_FINALIZE_BENCHMARK

void XORStateImpl::getInitialState(
    ProdState& p, const Marking& m) const
{
//  smoc::Detail::outDbg << "XORStateImpl::getInitialState(p,m) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

  HierarchicalStateImpl* t = 0;

  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    if((*s)->isMarked(m)) {
      if(t)
        throw ModelingError("smoc_xor_state: Must specify single initial state");
      t = *s;
    }
  }

  if(!t) t = init;
  assert(t);
  t->getInitialState(p, m);
}

const HierarchicalStateImpl* XORStateImpl::getTopState(
    const MultiState& d,
    bool isSrcState) const
{
  for(MultiState::const_iterator s = d.begin();
      s != d.end(); ++s)
  {
    if(!isAncestor(*s)) {
      assert(getParent());
      return getParent()->getTopState(d, false);
    }
  }
  
  return this;
}

void intrusive_ptr_add_ref(XORStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(XORStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }



ANDStateImpl::ANDStateImpl(const std::string& name)
  : HierarchicalStateImpl(name)
{}

void ANDStateImpl::add(HierarchicalStateImpl* part) {
  HierarchicalStateImpl::add(part);
}

void ANDStateImpl::getInitialState(
    ProdState& p, const Marking& m) const
{
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    (*s)->getInitialState(p, m);
  }
}

#ifdef FSM_FINALIZE_BENCHMARK
void ANDStateImpl::countStates(size_t& nLeaf, size_t& nAnd, size_t& nXor, size_t& nTrans) const {
  nAnd++;
  HierarchicalStateImpl::countStates(nLeaf, nAnd, nXor, nTrans);
}
#endif // FSM_FINALIZE_BENCHMARK

const HierarchicalStateImpl* ANDStateImpl::getTopState(
    const MultiState& d,
    bool isSrcState) const
{
  for(MultiState::const_iterator s = d.begin();
      s != d.end(); ++s)
  {
    if(!isAncestor(*s)) {
      assert(getParent());
      return getParent()->getTopState(d, false);
    }
    else if(*s == this) {
      isSrcState = true;
    }
  }

  if(isSrcState)
    return this;

  // if we come here, user tried to cross a partition
  // boundary without leaving the AND state

  throw ModelingError("smoc_and_state: Can't create inter-partition transitions");
}

void intrusive_ptr_add_ref(ANDStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(ANDStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }




JunctionStateImpl::JunctionStateImpl()
  : FiringStateBaseImpl() {}

void JunctionStateImpl::expandTransition(
    ExpandedTransitionList& etl,
    const ExpandedTransition& t) const
{
//  smoc::Detail::outDbg << "JunctionStateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

  assert(t.getDestStates().empty());

  if(ptl.empty()) {
    throw ModelingError("smoc_junction_state: Must specify at least one transition");
  }

  for(PartialTransitionList::const_iterator pt = ptl.begin();
      pt != ptl.end(); ++pt)
  {
    assert(pt->getDestState());
    pt->getDestState()->expandTransition(
        etl,
        ExpandedTransition(
          t.getSrcState(),
          t.getCondStates(),
          t.getExpr() && pt->getExpr(),
          merge(t.getAction(), pt->getAction())));
  }
}

void intrusive_ptr_add_ref(JunctionStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(JunctionStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }



MultiStateImpl::MultiStateImpl()
  : FiringStateBaseImpl() {}

void MultiStateImpl::finalise(ExpandedTransitionList& etl) {
//  smoc::Detail::outDbg << "MultiStateImpl::finalise(etl) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);
 
  // target state if no transitions
  if(ptl.empty())
    return;

  // at least one outgoing transition --> single src state
  if(!single(states)) {
    throw ModelingError("smoc_multi_state: Must specify single source state");
  }

  for(PartialTransitionList::const_iterator pt = ptl.begin();
      pt != ptl.end(); ++pt)
  {
    assert(pt->getDestState());
    pt->getDestState()->expandTransition(
        etl,
        ExpandedTransition(
          *states.begin(),
          condStates,
          pt->getExpr(), pt->getAction()));
  }
}

void MultiStateImpl::expandTransition(
    ExpandedTransitionList& etl,
    const ExpandedTransition& t) const
{
//  smoc::Detail::outDbg << "MultiStateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);
  
  assert(t.getDestStates().empty());

  if(states.empty()) {
    throw ModelingError("smoc_multi_state: Must specify at least one target state");
  }

  etl.push_back(
      ExpandedTransition(
        t.getSrcState(),
        t.getCondStates(),
        t.getExpr(),
        t.getAction(),
        states));
}


void MultiStateImpl::addState(HierarchicalStateImpl* s) {
//  smoc::Detail::outDbg << "MultiStateImpl::addState(s) this == " << this << std::endl; 
//  ScopedIndent s0(smoc::Detail::outDbg);

//  smoc::Detail::outDbg << "state: " << s << std::endl;
  
  fsm->unify(s->getFiringFSM());

  sassert(states.insert(s).second);
}

void MultiStateImpl::addCondState(HierarchicalStateImpl* s, bool neg) {
//  smoc::Detail::outDbg << "MultiStateImpl::addCondState(s) this == " << this << std::endl;
//  ScopedIndent s0(smoc::Detail::outDbg);

//  smoc::Detail::outDbg << "state: " << s << std::endl;
  
  fsm->unify(s->getFiringFSM());

  sassert(condStates.insert(std::make_pair(s, neg)).second);
}

void intrusive_ptr_add_ref(MultiStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(MultiStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }



smoc_firing_state_base::smoc_firing_state_base(const SmartPtr &p)
  : FFType(p) {}

void smoc_firing_state_base::addTransition(const smoc_transition_list &tl)
  { getImpl()->addTransition(tl); }

void smoc_firing_state_base::clearTransition()
  { getImpl()->clearTransition(); }

smoc_firing_state_base::ImplType *smoc_firing_state_base::getImpl() const 
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }

smoc_firing_state_base& smoc_firing_state_base::operator = (const smoc_transition_list &tl) {
  getImpl()->clearTransition();
  getImpl()->addTransition(tl);
  return *this;
}

smoc_firing_state_base& smoc_firing_state_base::operator |= (const smoc_transition_list &tl) {
  getImpl()->addTransition(tl);
  return *this;
}





smoc_hierarchical_state::smoc_hierarchical_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_hierarchical_state::ImplType *smoc_hierarchical_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }
  
smoc_hierarchical_state::Ref smoc_hierarchical_state::select(
    const std::string& name)
  { return smoc_hierarchical_state(PHierarchicalStateImpl(getImpl()->select(name))); }

smoc_hierarchical_state::ConstRef smoc_hierarchical_state::select(
    const std::string& name) const
  { return smoc_hierarchical_state(PHierarchicalStateImpl(getImpl()->select(name))); }
  
const std::string& smoc_hierarchical_state::getName() const
  { return getImpl()->getName(); }

std::string smoc_hierarchical_state::getHierarchicalName() const
  { return getImpl()->getHierarchicalName(); }

#ifdef SYSTEMOC_ENABLE_MAESTRO
/**
* @rosales: Clone method to enable the reassigment of the initial state
*			Rationale: States have a overloaded assignment operator
*/
smoc_hierarchical_state& smoc_hierarchical_state::clone(const smoc_hierarchical_state &st) {

	HierarchicalStateImpl* copyImp = st.getImpl();
	HierarchicalStateImpl* thisImp = this->getImpl();

	*thisImp = *copyImp;
	this->pImpl = st.pImpl;


	return *this;
}
#endif



smoc_firing_state::smoc_firing_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_firing_state::smoc_firing_state(const std::string& name)
  : FFType(new FiringStateImpl(name)) {}

smoc_firing_state::ImplType *smoc_firing_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }




smoc_xor_state::smoc_xor_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_xor_state::smoc_xor_state(const std::string& name)
  : FFType(new XORStateImpl(name)) {}

smoc_xor_state::smoc_xor_state(const smoc_hierarchical_state& i)
  : FFType(new XORStateImpl()) { init(i); }

smoc_xor_state::ImplType *smoc_xor_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }

smoc_xor_state& smoc_xor_state::init(const smoc_hierarchical_state& state)
  { getImpl()->add(state.getImpl(), true); return *this; }

smoc_xor_state& smoc_xor_state::add(const smoc_hierarchical_state& state)
  { getImpl()->add(state.getImpl(), false); return *this; }





smoc_and_state::smoc_and_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_and_state::smoc_and_state(const std::string& name)
  : FFType(new ANDStateImpl(name)) {}

smoc_and_state::ImplType *smoc_and_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }

smoc_and_state& smoc_and_state::add(const smoc_hierarchical_state& state)
  { getImpl()->add(state.getImpl()); return *this; }




smoc_junction_state::smoc_junction_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_junction_state::smoc_junction_state()
  : FFType(new JunctionStateImpl()) {}

smoc_junction_state::ImplType *smoc_junction_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }





smoc_multi_state::smoc_multi_state(const SmartPtr &p)
  : FFType(_StorageType(p)) {}

smoc_multi_state::smoc_multi_state(const smoc_hierarchical_state& s)
  : FFType(new MultiStateImpl()) { getImpl()->addState(s.getImpl()); }

smoc_multi_state::smoc_multi_state(const IN& s)
  : FFType(new MultiStateImpl()) { getImpl()->addCondState(s.s.getImpl(), s.neg); }

smoc_multi_state::ImplType *smoc_multi_state::getImpl() const
  { return CoSupport::DataTypes::FacadeCoreAccess::getImpl(*this); }

smoc_multi_state& smoc_multi_state::operator,(const smoc_hierarchical_state& s)
  { getImpl()->addState(s.getImpl()); return *this; }

smoc_multi_state& smoc_multi_state::operator,(const IN& s)
  { getImpl()->addCondState(s.s.getImpl(), s.neg); return *this; }

