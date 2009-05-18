// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include <CoSupport/SmartPtr/RefCountObject.hpp>
#include <CoSupport/DataTypes/oneof.hpp>
#include <CoSupport/String/Concat.hpp>
#include <CoSupport/Math/flog2.hpp>

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_node_types.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/hscd_tdsim_TraceLog.hpp>
#include <systemoc/smoc_ngx_sync.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <systemoc/detail/smoc_firing_rules_impl.hpp>

#include <sgx.hpp>

using namespace CoSupport::DataTypes;
using namespace SysteMoC::NGXSync;
using namespace SystemCoDesigner::SGX;

#include <CoSupport/Streams/FilterOStream.hpp>
#include <CoSupport/Streams/IndentStreambuf.hpp>
using namespace CoSupport::Streams;
struct MyDebugOstream : public FilterOStream {
  IndentStreambuf i;
  MyDebugOstream()
    : FilterOStream(std::cerr) { insert(i); }
};
MyDebugOstream outDbg;

// Prints duration of FiringFSMImpl::finalise() in secs.
//#define FSM_FINALIZE_BENCHMARK

static const char HIERARCHY_SEPARATOR = '.';


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
    const smoc_activation_pattern& ap,
    const smoc_action& f)
  : ap(ap),
    f(f)
{}

const smoc_activation_pattern& TransitionBase::getActivationPattern() const
  { return ap; }

const smoc_action& TransitionBase::getAction() const
  { return f; }





PartialTransition::PartialTransition(
    const smoc_activation_pattern& ap,
    const smoc_action& f,
    FiringStateBaseImpl* dest)
  : TransitionBase(ap, f),
    dest(dest)
{}

FiringStateBaseImpl* PartialTransition::getDestState() const
  { return dest; }


  

ExpandedTransition::ExpandedTransition(
    const HierarchicalStateImpl* src,
    const CondMultiState& in,
    const smoc_activation_pattern& ap,
    const smoc_action& f,
    const MultiState& dest)
  : TransitionBase(ap, f),
    src(src),
    in(in),
    dest(dest)
{}

ExpandedTransition::ExpandedTransition(
    const HierarchicalStateImpl* src,
    const CondMultiState& in,
    const smoc_activation_pattern& ap,
    const smoc_action& f)
  : TransitionBase(ap, f),
    src(src),
    in(in)
{}

ExpandedTransition::ExpandedTransition(
    const HierarchicalStateImpl* src,
    const smoc_activation_pattern& ap,
    const smoc_action& f)
  : TransitionBase(ap, f),
    src(src)
{}

const HierarchicalStateImpl* ExpandedTransition::getSrcState() const
  { return src; }

const CondMultiState& ExpandedTransition::getCondStates() const
  { return in; }

const MultiState& ExpandedTransition::getDestStates() const
  { return dest; }





RuntimeTransition::RuntimeTransition(
    smoc_root_node* actor,
    const smoc_activation_pattern& ap,
    const smoc_action& f,
    RuntimeState* dest)
  : smoc_activation_pattern(ap),
    actor(actor),
    f(f),
    dest(dest)
{
  // if this breaks everything, remove it
  finalise();
}

#ifdef SYSTEMOC_DEBUG
Expr::Detail::ActivationStatus RuntimeTransition::getStatus() const {
  std::cerr << "RuntimeTransition::getStatus: " << *this << std::endl;
  return smoc_activation_pattern::getStatus();
}
#endif

bool RuntimeTransition::isEnabled() const
  { return getStatus() == Expr::Detail::ENABLED(); }

smoc_root_node &RuntimeTransition::getActor()
  { assert(actor != NULL); return *actor; }

void RuntimeTransition::execute(int mode) {
  enum {
    MODE_DIISTART,
    MODE_DIIEND,
    MODE_GRAPH
  } execMode;
  
  if(dynamic_cast<smoc_graph_base*>(actor) == NULL) {
    execMode =
#ifdef SYSTEMOC_ENABLE_VPC
      actor->getCurrentState() != actor->getCommState()
        ? MODE_DIISTART
        : MODE_DIIEND;
#else
    MODE_DIISTART;
#endif
  }
  else {
#ifdef SYSTEMOC_ENABLE_VPC
    assert(actor->getCurrentState() != actor->getCommState());
#endif
    execMode = MODE_GRAPH;
  }

#ifdef SYSTEMOC_DEBUG
  static const char *execModeName[] = { "diiStart", "diiEnd", "graph" };

  std::cerr << "  <transition actor=\"" << actor->name()
            << "\" mode=\"" << execModeName[execMode]
            << "\">" << std::endl;
#endif

#ifdef SYSTEMOC_TRACE
  if(execMode != MODE_GRAPH)
    TraceLog.traceStartActor(actor, execMode == MODE_DIISTART ? "s" : "e");
#endif

#if defined(SYSTEMOC_ENABLE_DEBUG) || defined(SYSTEMOC_TRACE)
  Expr::evalTo<Expr::CommSetup>(guard);
#endif

  // only smoc_func_diverge may set nextState to something
  // different than dest here...
  RuntimeState* nextState =
    boost::apply_visitor(ActionVisitor(dest, mode), f);

#if defined(SYSTEMOC_ENABLE_DEBUG)
  Expr::evalTo<Expr::CommReset>(guard);
#endif

#ifdef SYSTEMOC_ENABLE_VPC
  if(execMode == MODE_DIISTART /*&& (mode&GO)*/) {
    actor->diiEvent->reset();
    smoc_ref_event_p latEvent(new smoc_ref_event());
    
    SystemC_VPC::EventPair p(actor->diiEvent.get(), latEvent.get());

    // new FastLink interface
    if(mode & GO) {
      vpcLink->compute(p);
    }
    else if(mode & TICK) {
      smoc_sr_func_pair* fp = boost::get<smoc_sr_func_pair>(&f);
      assert(fp);
      fp->tickLink->compute(p);
    }

    // save nextState to later execute communication
    actor->setNextState(nextState);

    // insert magic commstate
    nextState = actor->getCommState();
    Expr::evalTo<Expr::CommExec>(guard, actor->diiEvent, latEvent);

    // This covers the case that the executed transition does not
    // contain an output port. Therefore, the latEvent is not added
    // to a LatencyQueue and would be deleted immediately after
    // the latEvent smartptr is destroyed when this scope is left.
    if(!*latEvent) {
      // latency event not signaled
      struct _: public smoc_event_listener {
        smoc_ref_event_p  latEvent;
        smoc_root_node   *actor;
        
        void signaled(smoc_event_waiter *_e) {
# ifdef SYSTEMOC_TRACE
  //      const char *name = actor->name();
          
          TraceLog.traceStartActor(actor, "l");
# endif
# ifdef SYSTEMOC_DEBUG
          std::cerr << "smoc_root_node::_communicate::_::signaled(...)"
                    << std::endl;
# endif
          assert(_e == &*latEvent);
          assert(*_e);
          latEvent = NULL;
# ifdef SYSTEMOC_TRACE
          TraceLog.traceEndActor(actor);
# endif
          return;
        }
        void eventDestroyed(smoc_event_waiter *_e) {
# ifdef SYSTEMOC_DEBUG
          std::cerr << "smoc_root_node::_communicate::_:: eventDestroyed(...)"
                    << std::endl;
# endif
          delete this;
        }
        
        _(const smoc_ref_event_p &latEvent, smoc_root_node *actor)
          : latEvent(latEvent), actor(actor) {};
        
        virtual ~_() {}
      };
      latEvent->addListener(new _(latEvent, actor));
    }
    else {
# ifdef SYSTEMOC_TRACE
      TraceLog.traceStartActor(this, "l");
      TaceLog.traceEndActor(this);
# endif
    }
  }
  else {
    Expr::evalTo<Expr::CommExec>(guard, NULL, NULL);
  }
#else // SYSTEMOC_ENABLE_VPC
  Expr::evalTo<Expr::CommExec>(guard);
#endif // SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_TRACE
  if(execMode != MODE_GRAPH)
    TraceLog.traceEndActor(actor);
#endif

  actor->setCurrentState(nextState);

#ifdef SYSTEMOC_DEBUG
  std::cerr << "  </transition>"<< std::endl;
#endif
}

void RuntimeTransition::finalise() {
  assert(actor != NULL);

  smoc_activation_pattern::finalise();

#ifdef SYSTEMOC_ENABLE_VPC
  if(dynamic_cast<smoc_actor *>(actor) != NULL) {
    vpcLink = boost::apply_visitor(
        VPCLinkVisitor(actor->name()), f);
  }
#endif //SYSTEMOC_ENABLE_VPC
}

RuntimeState* RuntimeTransition::getDestState() const
  { return dest; }

const smoc_action& RuntimeTransition::getAction() const
  { return f; }

//static int RuntimeStateCount = 0;

RuntimeState::RuntimeState()
  : sc_object(sc_gen_unique_name("smoc_firing_state"))
  //: sc_object(CoSupport::String::Concat
  //    ("smoc_firing_state_")(RuntimeStateCount++).get().c_str())
{
  idPool.regObj(this);
#ifndef __SCFE__
  assembleXML();
#endif
}

RuntimeState::~RuntimeState()
  { idPool.unregObj(this); }

#ifndef __SCFE__
void RuntimeState::assembleXML() {
  assert(!state);

  FiringState _state(name());
  state = &_state;
}
#endif

const RuntimeTransitionList& RuntimeState::getTransitions() const
  { return t; }

RuntimeTransitionList& RuntimeState::getTransitions()
  { return t; }

FiringState::Ptr RuntimeState::getState() const
  { return state; }



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

//smoc_root_node* FiringFSMImpl::getActor() const
//  { assert(actor != NULL); return actor; }

const RuntimeStateSet& FiringFSMImpl::getStates() const
  { return rts; }

RuntimeState* FiringFSMImpl::getInitialState() const
  { return init; }
  
FiringFSM::Ptr FiringFSMImpl::getFSM() const
  { return fsm; }

std::ostream& operator<<(std::ostream& os, const ProdState& p) {
  os << "(";
  for(ProdState::const_iterator s = p.begin();
      s != p.end(); ++s)
  {
    if(s != p.begin()) os << ",";
    
    if((*s)->getName().empty())
      os << *s;
    else
      os << (*s)->getName();
  }
  return os << ")";
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

void FiringFSMImpl::finalise(
    smoc_root_node* actor,
    HierarchicalStateImpl* hsinit)
{
//  outDbg << "FiringFSMImpl::finalise(...) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);

  assert(actor);
  //actor = _actor;

//  outDbg << "Actor: " << actor->name() << std::endl;

#ifdef FSM_FINALIZE_BENCHMARK  
  clock_t finStart;
#endif // FSM_FINALIZE_BENCHMARK

  try {

    // create top state (do not try this in the constructor...)
    top = new XORStateImpl();
    unify(top->getFiringFSM());
    delState(top);

//    outDbg << "#states: " << states.size() << std::endl;

    // move remaining hierarchical states into top state
//    {outDbg << "moving HS to top state" << std::endl;
//    ScopedIndent s1(outDbg);
    
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

    // finalise states -> expanded transitions
//    {outDbg << "finalising states" << std::endl;
//    ScopedIndent s1(outDbg);
    
    for(FiringStateBaseImplSet::iterator sIter = states.begin();
        sIter != states.end(); ++sIter)
    {
      (*sIter)->finalise(etl);
    }

    top->setCodeAndBits(0,1);
    top->finalise(etl);

//    }

    // calculate runtime states and transitions
//    {outDbg << "calculating runtime states / transitions" << std::endl;
//    ScopedIndent s1(outDbg);

#ifdef FSM_FINALIZE_BENCHMARK  
   finStart = clock();
#endif // FSM_FINALIZE_BENCHMARK

    assert(rts.empty());

    typedef std::map<ProdState,RuntimeState*> StateTable;
    typedef StateTable::value_type STEntry;
    StateTable st;

    typedef std::list<StateTable::const_iterator> NewStates;
    NewStates ns;

    // determine initial state
    ProdState psinit;
    top->getInitialState(psinit, Marking());

    init = *rts.insert(new RuntimeState()).first;
    ns.push_back(
        st.insert(STEntry(psinit, init)).first);

    std::ofstream* fsmDump = 0;

    if(smoc_modes::dumpFSMs) {

      std::string f =
        CoSupport::String::Concat("FSM_")(actor->name())(".dot");
      fsmDump = new std::ofstream(f.c_str());

      *fsmDump << "digraph G {" << std::endl;
    }

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
            st.insert(STEntry(d, 0));
                
          if(ins.second) {
            // FIXME: construct state name and pass to RuntimeState
            ins.first->second =
              *rts.insert(new RuntimeState()).first;  
            ns.push_back(ins.first);
          }

          RuntimeState* rd = ins.first->second;
          assert(rd);

          if(smoc_modes::dumpFSMs) {
            *fsmDump << '"' << s << '"' << " -> "
                     << '"' << d << '"' << std::endl;
          }

          // create runtime transition
//          outDbg << "creating runtime transition " << rs << " -> " << rd << std::endl;

          rs->getTransitions().push_back(
              RuntimeTransition(
                actor,
                t->getActivationPattern(),
                t->getAction(),
                rd));
        }
      }
    }

    if(smoc_modes::dumpFSMs) {
      *fsmDump << "}" << std::endl;
      fsmDump->close();
      delete fsmDump;
    }     


//    }
  }
  catch(const ModelingError& e) {
    std::cerr << "A modeling error occurred:" << std::endl
              << "  \"" << e.what() << "\"" << std::endl
              << "Please check the model and try again!" << std::endl;
    // give up
    exit(1);
  }

#ifdef FSM_FINALIZE_BENCHMARK  
  outDbg << "Finalised FSM of actor '" << actor->name() << "' in "
         << ((clock() - finStart) / (double)CLOCKS_PER_SEC) << " secs."
         << std::endl;
#endif // FSM_FINALIZE_BENCHMARK

#ifndef __SCFE__
  assembleXML();
#endif
}

#ifndef __SCFE__
void FiringFSMImpl::assembleXML() {
  assert(!fsm);

  FiringFSM _fsm;
  fsm = &_fsm;

  for(RuntimeStateSet::const_iterator sIter = rts.begin();
      sIter != rts.end(); ++sIter)
  {
    FiringState::Ptr src = (*sIter)->getState();
    fsm->states().push_back(*src);

    if(*sIter == init)
      fsm->startState() = src;

    const RuntimeTransitionList& tList = (*sIter)->getTransitions();

    for(RuntimeTransitionList::const_iterator tIter = tList.begin();
        tIter != tList.end(); ++tIter)
    {
      FiringTransition t;
      t.dstState() = tIter->getDestState()->getState();
      src->outTransitions().push_back(t);
    }
  }
}
#endif

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
          st->getActivationPattern(),
          st->getInterfaceAction().getAction(),
          st->getInterfaceAction().getDestState().getImpl()));
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


void intrusive_ptr_add_ref(FiringStateBaseImpl *p)
  { p->getFiringFSM()->addRef(); }

void intrusive_ptr_release(FiringStateBaseImpl *p)
  { if(p->getFiringFSM()->delRef()) delete p->getFiringFSM(); }





HierarchicalStateImpl::HierarchicalStateImpl(const std::string& name)
  : FiringStateBaseImpl(),
    name(name),
    parent(0)
{
  if(name.find(HIERARCHY_SEPARATOR) != std::string::npos)
    assert(!"smoc_hierarchical_state: Invalid state name");
}

HierarchicalStateImpl::~HierarchicalStateImpl()
  {}

const std::string& HierarchicalStateImpl::getName() const
  { return name; }

std::string HierarchicalStateImpl::getHierarchicalName() const {
  if(!parent || parent == fsm->top) {
    return name;
  }
  return
    parent->getHierarchicalName() +
    HIERARCHY_SEPARATOR +
    (name.empty() ? "???" : name) ;
}

void HierarchicalStateImpl::setParent(HierarchicalStateImpl* v) {
  assert(v);
  if(parent && v != parent) {
    assert(!"smoc_hierarchical_state: Parent already set");
  }
  parent = v;
}

bool HierarchicalStateImpl::isAncestor(const HierarchicalStateImpl* s) const {
  assert(s);

  if(s == this)
    return true;

  if(s->bits > bits)
    return (code == (s->code >> (s->bits - bits)));
  
  return false;
}
  
void HierarchicalStateImpl::setCodeAndBits(uint64_t c, size_t b) {
  assert(b < 64);
  code = c;
  //mask = (~0ull)<<b;
  bits = b;
}

size_t HierarchicalStateImpl::getChildCodeAndBits(size_t cs, uint64_t& cc) const {
  assert(cs);

  size_t cb = (cs == 1) ? 1 : CoSupport::flog2(static_cast<uint32_t>(cs) - 1);
  assert(bits + cb <= 64);

  //outDbg << "#C: " << cs << " -> CB: " << cb << std::endl;
            
  cc = code << cb;
  return cb + bits;
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
//  outDbg << "HierarchicalStateImpl::finalise(etl) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);
  
//  outDbg << "State: " << getName() << "; Code: " << code << "; Bits: " << bits << std::endl;

  for(PartialTransitionList::const_iterator pt = ptl.begin();
      pt != ptl.end(); ++pt)
  {
    assert(pt->getDestState());
    pt->getDestState()->expandTransition(
        etl,
        ExpandedTransition(
          this, pt->getActivationPattern(), pt->getAction()));
  }
}

void HierarchicalStateImpl::expandTransition(
    ExpandedTransitionList& etl,
    const ExpandedTransition& t) const
{
//  outDbg << "HierarchicalStateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);
  
  assert(t.getDestStates().empty());

  MultiState dest;
  dest.insert(this);

  etl.push_back(
      ExpandedTransition(
        t.getSrcState(),
        t.getCondStates(),
        t.getActivationPattern(),
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
//  outDbg << "FiringStateImpl::getInitialState(p,m) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);

  p.insert(this);
}

const HierarchicalStateImpl* FiringStateImpl::getTopState(
    const MultiState& d,
    bool isSrcState) const
{
  for(MultiState::const_iterator s = d.begin();
      s != d.end(); ++s)
  {
    if(*s != this) {
      assert(parent);
      return parent->getTopState(d, false);
    }
  }

  return this;
}

HierarchicalStateImpl* FiringStateImpl::select(
    const std::string& name)
{
  if(!name.empty())
    assert(!"smoc_firing_state: No child states");

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

XORStateImpl::~XORStateImpl() {
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    assert((*s)->getFiringFSM() == fsm);
    delete *s;
  }
}

void XORStateImpl::add(HierarchicalStateImpl* state, bool i) {
//  outDbg << "XORStateImpl::add(...) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);
  
//  outDbg << "state: " << state << "; init: " << i << std::endl;
  
  fsm->unify(state->getFiringFSM());
  fsm->delState(state);
  
  c.insert(state);
  state->setParent(this);
  
  if(i) init = state;
}

void XORStateImpl::setFiringFSM(FiringFSMImpl *fsm) {
  FiringStateBaseImpl::setFiringFSM(fsm);
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    (*s)->setFiringFSM(fsm);
  }
}
  
void XORStateImpl::finalise(ExpandedTransitionList& etl) {
//  ioutDbg << "XORStateImpl::finalise(etl) this == " << this << std::endl;
  ScopedIndent s0(outDbg);

//  outDbg << "name: " << name << std::endl;

  if(!init)
    throw ModelingError("smoc_xor_state: Must specify initial state");

//  std::cout << "State: " << getName() << "; Code: " << code << "; Bits: " << bits << std::endl;

  if(!c.empty()) {
    uint64_t cc; 
    size_t cb = getChildCodeAndBits(c.size(), cc);

    for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
      (*s)->setCodeAndBits(cc, cb);
      (*s)->finalise(etl);
      ++cc;
    }
  }

  HierarchicalStateImpl::finalise(etl);
}

void XORStateImpl::getInitialState(
    ProdState& p, const Marking& m) const
{
//  outDbg << "XORStateImpl::getInitialState(p,m) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);

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
      assert(parent);
      return parent->getTopState(d, false);
    }
  }
  
  return this;
}

HierarchicalStateImpl* XORStateImpl::select(
    const std::string& name)
{
  size_t pos = name.find(HIERARCHY_SEPARATOR);
  std::string top = name.substr(0, pos);

  if(top.empty()) {
    if(pos == std::string::npos)
      return this;
    else
      assert(!"smoc_xor_state: Invalid hierarchical name");
  }
 
  for(C::iterator s = c.begin(); s != c.end(); ++s) {
    if((*s)->getName() == top) {
      if(pos == std::string::npos)
        return (*s);
      else
        return (*s)->select(name.substr(pos + 1));
    }
  }
  
  assert(!"smoc_xor_state: Invalid hierarchical name");
}

void intrusive_ptr_add_ref(XORStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(XORStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }



ANDStateImpl::ANDStateImpl(size_t part, const std::string& name)
  : HierarchicalStateImpl(name),
    c(part)
{
  if(part < 2)
    assert(!"smoc_and_state: Must contain at least two partitions");
  for(size_t i = 0; i < part; ++i) {
    c[i] = new XORStateImpl(CoSupport::String::Concat(i));

    fsm->unify(c[i]->getFiringFSM());
    fsm->delState(c[i]);

    c[i]->setParent(this);
  }
}

ANDStateImpl::~ANDStateImpl() {
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    assert((*s)->getFiringFSM() == fsm);
    delete *s;
  }
}

void ANDStateImpl::setFiringFSM(FiringFSMImpl *fsm) {
  FiringStateBaseImpl::setFiringFSM(fsm);
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    (*s)->setFiringFSM(fsm);
  }
}

void ANDStateImpl::finalise(ExpandedTransitionList& etl) {
//  outDbg << "ANDStateImpl::finalise(etl) this == " << this << std::endl;
  ScopedIndent s0(outDbg);
  
//  std::cout << "State: " << getName() << "; Code: " << code << "; Bits: " << bits << std::endl;

  if(!c.empty()) {
    uint64_t cc; 
    size_t cb = getChildCodeAndBits(c.size(), cc);

    for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
      (*s)->setCodeAndBits(cc, cb);
      (*s)->finalise(etl);
      ++cc;
    }
  }

  HierarchicalStateImpl::finalise(etl);
}

void ANDStateImpl::getInitialState(
    ProdState& p, const Marking& m) const
{
//  outDbg << "ANDStateImpl::getInitialState(p,m) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);
  
  for(C::const_iterator s = c.begin(); s != c.end(); ++s) {
    (*s)->getInitialState(p, m);
  }
}

const HierarchicalStateImpl* ANDStateImpl::getTopState(
    const MultiState& d,
    bool isSrcState) const
{
  for(MultiState::const_iterator s = d.begin();
      s != d.end(); ++s)
  {
    if(!isAncestor(*s)) {
      assert(parent);
      return parent->getTopState(d, false);
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

XORStateImpl* ANDStateImpl::getPart(size_t p) const {
  if(p >= c.size())
    assert(!"smoc_and_state: Invalid partition");
  return c[p];
}

HierarchicalStateImpl* ANDStateImpl::select(
    const std::string& name)
{
  size_t pos = name.find(HIERARCHY_SEPARATOR);
  std::string top = name.substr(0, pos);

  if(top.empty()) {
    if(pos == std::string::npos)
      return this;
    else
      assert(!"smoc_and_state: Invalid hierarchical name");
  }
 
  for(C::iterator s = c.begin(); s != c.end(); ++s) {
    if((*s)->getName() == top) {
      if(pos == std::string::npos)
        return (*s);
      else
        return (*s)->select(name.substr(pos + 1));
    }
  }
  
  assert(!"smoc_and_state: Invalid hierarchical name");
}

void intrusive_ptr_add_ref(ANDStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(ANDStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }




ConnectorStateImpl::ConnectorStateImpl()
  : FiringStateBaseImpl() {}

void ConnectorStateImpl::expandTransition(
    ExpandedTransitionList& etl,
    const ExpandedTransition& t) const
{
//  outDbg << "ConnectorStateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);

  assert(t.getDestStates().empty());

  if(ptl.empty()) {
    throw ModelingError("smoc_connector_state: Must specify at least one transition");
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
          t.getActivationPattern().getExpr() && pt->getActivationPattern().getExpr(),
          merge(t.getAction(), pt->getAction())));
  }
}

void intrusive_ptr_add_ref(ConnectorStateImpl *p)
  { intrusive_ptr_add_ref(static_cast<FiringStateBaseImpl*>(p)); }

void intrusive_ptr_release(ConnectorStateImpl *p)
  { intrusive_ptr_release(static_cast<FiringStateBaseImpl*>(p)); }



MultiStateImpl::MultiStateImpl()
  : FiringStateBaseImpl() {}

void MultiStateImpl::finalise(ExpandedTransitionList& etl) {
//  outDbg << "MultiStateImpl::finalise(etl) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);
 
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
          pt->getActivationPattern(), pt->getAction()));
  }
}

void MultiStateImpl::expandTransition(
    ExpandedTransitionList& etl,
    const ExpandedTransition& t) const
{
//  outDbg << "MultiStateImpl::expandTransition(etl,t) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);
  
  assert(t.getDestStates().empty());

  if(states.empty()) {
    throw ModelingError("smoc_multi_state: Must specify at least one target state");
  }

  etl.push_back(
      ExpandedTransition(
        t.getSrcState(),
        t.getCondStates(),
        t.getActivationPattern(),
        t.getAction(),
        states));
}


void MultiStateImpl::addState(HierarchicalStateImpl* s) {
//  outDbg << "MultiStateImpl::addState(s) this == " << this << std::endl; 
//  ScopedIndent s0(outDbg);

//  outDbg << "state: " << s << std::endl;
  
  fsm->unify(s->getFiringFSM());

  sassert(states.insert(s).second);
}

void MultiStateImpl::addCondState(HierarchicalStateImpl* s, bool neg) {
//  outDbg << "MultiStateImpl::addCondState(s) this == " << this << std::endl;
//  ScopedIndent s0(outDbg);

//  outDbg << "state: " << s << std::endl;
  
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
  : FFType(p) {}

smoc_hierarchical_state::ImplType *smoc_hierarchical_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }
  
smoc_hierarchical_state::Ref smoc_hierarchical_state::select(
    const std::string& name)
  { return smoc_hierarchical_state(getImpl()->select(name)); }

smoc_hierarchical_state::ConstRef smoc_hierarchical_state::select(
    const std::string& name) const
  { return smoc_hierarchical_state(getImpl()->select(name)); }
  
const std::string& smoc_hierarchical_state::getName() const
  { return getImpl()->getName(); }

std::string smoc_hierarchical_state::getHierarchicalName() const
  { return getImpl()->getHierarchicalName(); }




smoc_firing_state::smoc_firing_state(const SmartPtr &p)
  : FFType(p) {}

smoc_firing_state::smoc_firing_state(const std::string& name)
  : FFType(new FiringStateImpl(name)) {}

smoc_firing_state::ImplType *smoc_firing_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }




smoc_xor_state::smoc_xor_state(const SmartPtr &p)
  : FFType(p) {}

smoc_xor_state::smoc_xor_state(const std::string& name)
  : FFType(new XORStateImpl(name)) {}

smoc_xor_state::smoc_xor_state(const smoc_hierarchical_state& i)
  : FFType(new XORStateImpl()) { init(i); }

smoc_xor_state::ImplType *smoc_xor_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }

smoc_xor_state& smoc_xor_state::init(const smoc_hierarchical_state& state)
  { getImpl()->add(state.getImpl(), true); return *this; }

smoc_xor_state& smoc_xor_state::add(const smoc_hierarchical_state& state)
  { getImpl()->add(state.getImpl(), false); return *this; }





smoc_and_state::smoc_and_state(const SmartPtr &p)
  : FFType(p) {}

smoc_and_state::smoc_and_state(size_t part, const std::string& name)
  : FFType(new ANDStateImpl(part, name)) {}

smoc_and_state::ImplType *smoc_and_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }

smoc_xor_state::Ref smoc_and_state::operator[](size_t p)
  { return smoc_xor_state(getImpl()->getPart(p)); }




smoc_connector_state::smoc_connector_state(const SmartPtr &p)
  : FFType(p) {}

smoc_connector_state::smoc_connector_state()
  : FFType(new ConnectorStateImpl()) {}

smoc_connector_state::ImplType *smoc_connector_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }





smoc_multi_state::smoc_multi_state(const SmartPtr &p)
  : FFType(p) {}

smoc_multi_state::smoc_multi_state(const smoc_hierarchical_state& s)
  : FFType(new MultiStateImpl()) { getImpl()->addState(s.getImpl()); }

smoc_multi_state::smoc_multi_state(const IN& s)
  : FFType(new MultiStateImpl()) { getImpl()->addCondState(s.s.getImpl(), s.neg); }

smoc_multi_state::ImplType *smoc_multi_state::getImpl() const
  { return static_cast<ImplType *>(this->pImpl.get()); }

smoc_multi_state& smoc_multi_state::operator,(const smoc_hierarchical_state& s)
  { getImpl()->addState(s.getImpl()); return *this; }

smoc_multi_state& smoc_multi_state::operator,(const IN& s)
  { getImpl()->addCondState(s.s.getImpl(), s.neg); return *this; }

