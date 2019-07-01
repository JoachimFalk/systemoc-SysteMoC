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

#include <smoc/SimulatorAPI/SchedulerInterface.hpp>

#include "FiringFSM.hpp"
#include "RuntimeState.hpp"
#include "RuntimeTransition.hpp"
#include "RuntimeFiringRule.hpp"
#include "XORStateImpl.hpp"
#include "FiringStateImpl.hpp"
#include "../SimulationContext.hpp"

#include <systemoc/smoc_config.h>

#include <CoSupport/String/Concat.hpp>

#include <cassert>
#include <iostream>

namespace smoc { namespace Detail { namespace FSM {

using CoSupport::String::Concat;

static
std::ostream &operator<<(std::ostream &os, ProdState const &p) {
  // We need to sort the names of the states in the prod state in order to
  // generate a deterministic prod state name. For this sorting, we need a
  // map due to corner case of identical state names. The integer is the
  // multiplicity of a state name. This should almost always be one.
  std::map<std::string, int> names;
  for (FiringStateImpl const *s : p) {
    assert(!s->getName().empty());
    std::string name(s->getHierarchicalName());
    assert(!name.empty());
    names[name]++;
  }
  bool first = true;
  for (std::map<std::string, int>::value_type &v : names) {
    for (; v.second; --v.second) {
      if (!first)
        os << FiringFSM::PRODSTATE_SEPARATOR;
      first = false;
      os << v.first;
    }
  }
  return os;
}

/*
 * isImplied(s,p) == true <=> exists p_i in p: isAncestor(s, p_i) == true
 */
static
bool isImplied(const StateImpl* s, const ProdState& p) {
  for(ProdState::const_iterator pIter = p.begin();
      pIter != p.end(); ++pIter)
  {
    if(s->isAncestor(*pIter))
      return true;
  }
  return false;
}

static
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

template<class T>
static
void markStates(const T& t, Marking& m)  {
  for(typename T::const_iterator tIter = t.begin();
      tIter != t.end(); ++tIter)
  {
    (*tIter)->mark(m);
  }
}

FiringFSM::FiringFSM()
  : use_count_(0)
  , top(nullptr)
  , init(nullptr)
{
//std::cerr << "FiringFSM::FiringFSM() this == " << this << std::endl;
}

FiringFSM::~FiringFSM() {
//std::cerr << "FiringFSM::~FiringFSM() this == " << this << std::endl;
  assert(use_count_ == 0);

  delete top;

  for(BaseStateImpl *s : states) {
    assert(s->getFiringFSM() == this);
    delete s;
  }
  for(RuntimeState *s : rts)
    delete s;
  for (RuntimeFiringRule *fr : firingRules)
    delete fr;
}

FiringFSM::RuntimeStateSet const &FiringFSM::getStates() const
  { return rts; }

RuntimeState *FiringFSM::getInitialState() const
  { return init; }

RuntimeFiringRule *FiringFSM::acquireFiringRule(smoc_firing_rule const &smocFiringRule) {
  firingRules.push_front(new RuntimeFiringRule(
      smocFiringRule.getGuard(), smocFiringRule.getAction()));
  return firingRules.front();
}

#ifdef SYSTEMOC_ENABLE_HOOKING
void FiringFSM::addTransitionHook(
  std::string const &srcStateRegex,
  std::string const &actionRegex,
  std::string const &dstStateRegex,
  smoc_pre_hook_callback  const &pre,
  smoc_post_hook_callback const &post)
{
  transitionHooks.push_back(
      RuntimeTransitionHook(srcStateRegex, actionRegex, dstStateRegex, pre, post));
}
#endif //SYSTEMOC_ENABLE_HOOKING

void FiringFSM::before_end_of_elaboration(NodeBase *node, StateImpl *hsinit) {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<FiringFSM::end_of_elaboration name=\"" << node->name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)

  assert(node);

//smoc::Detail::outDbg << "Actor or Graph: " << actorOrGraphNode->name() << std::endl;

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
    for(BaseStateImplSet::iterator sIter = states.begin();
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

    BaseStateImplSet::iterator sIter, sNext;

    bool initStateFound = false;

    for(sIter = states.begin();
        sIter != states.end(); sIter = sNext) {
      ++(sNext = sIter);
      if (StateImpl* hs =
          dynamic_cast<StateImpl*>(*sIter))
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
    for(BaseStateImplSet::iterator sIter = states.begin();
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
    init = *rts.insert(new RuntimeState (Concat("")(psinit), dynamic_cast<Bruckner::Model::Hierarchical*>(node) )).first;
#else //!defined(SYSTEMOC_ENABLE_MAESTRO) || !defined(MAESTRO_ENABLE_BRUCKNER)
    init = *rts.insert(new RuntimeState(Concat(node->name())(":")(psinit))).first;
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
          const StateImpl* x =
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
            ins.first->second = *rts.insert(new RuntimeState(Concat("")(f), dynamic_cast<Bruckner::Model::Hierarchical*>(node))).first;
#else //!defined(SYSTEMOC_ENABLE_MAESTRO) || !defined(MAESTRO_ENABLE_BRUCKNER)
            ins.first->second = *rts.insert(new RuntimeState(Concat(node->name())(":")(f))).first;
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
          if (node->isActor()) {
            a = dynamic_cast<MetaMap::SMoCActor*>(node);
          }
#endif //SYSTEMOC_ENABLE_MAESTRO
          rs->addTransition(
            RuntimeTransition(
#ifdef SYSTEMOC_ENABLE_HOOKING
              transitionHooks,
              rs,
#endif //SYSTEMOC_ENABLE_HOOKING
              t->getFiringRule(),
#ifdef SYSTEMOC_ENABLE_MAESTRO
              *a,
#endif //SYSTEMOC_ENABLE_MAESTRO
              rd));
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

  std::cerr << "Finalised FSM of actor/graph '" << node->name() << "' in "
            << ((finEnd - finStart) / 1e9) << "s"
            << "; #states: " << nRunStates << "; #trans: " << nRunTrans
            << std::endl;
#endif // FSM_FINALIZE_BENCHMARK

  getSimCTX()->getSimulatorInterface()->registerTask(node,
      reinterpret_cast<std::list<SimulatorAPI::FiringRuleInterface *> const &>(firingRules));
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</FiringFSM::end_of_elaboration>"
         << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

void FiringFSM::end_of_elaboration(NodeBase *node, StateImpl *hsinit) {
  for (RuntimeFiringRule *fr : firingRules)
    fr->end_of_elaboration();
  for (RuntimeState *s : rts)
    s->end_of_elaboration();
}

void FiringFSM::addState(BaseStateImpl *state) {
  assert(state->getFiringFSM() == this);
  sassert(states.insert(state).second);
}

void FiringFSM::delState(BaseStateImpl *state) {
  assert(state->getFiringFSM() == this);
  sassert(states.erase(state) == 1);
}

void FiringFSM::addRef() {
//std::cerr << "FiringFSM::add_ref() this == " << this
//          << "; use_count: " << use_count_ << std::endl;
  ++use_count_;
}

bool FiringFSM::delRef() {
//std::cerr << "FiringFSM::delRef() this == " << this
//          << "; use_count: " << use_count_ << std::endl;
  assert(use_count_); --use_count_;
  return use_count_ == 0;
}

void FiringFSM::unify(this_type *fr) {
  if (this != fr) {
//  std::cerr << "FiringFSM::unify() this == " << this
//            << "; other: " << fr << std::endl;

    // patch fsm of all states owned by fr
    for(BaseStateImplSet::iterator sIter = fr->states.begin();
        sIter != fr->states.end(); ++sIter)
    {
      assert((*sIter)->getFiringFSM() == fr);
      (*sIter)->setFiringFSM(this);
      sassert(states.insert(*sIter).second);
    }

//  std::cerr << " own use_count: " << use_count_
//            << "; other use_count: " << fr->use_count_
//            << "; # merged states: " << states.size()
//            << std::endl;

    use_count_ += fr->use_count_;
    fr->use_count_ = 0;
    fr->states.clear();

    delete fr;
  }
}

} } } // namespace smoc::Detail::FSM
