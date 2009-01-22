//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <boost/variant.hpp>
#include <boost/blank.hpp>

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_graph_synth.hpp>

namespace SysteMoC {

using namespace SGX;
using namespace NGXSync;

// find non-hierarchical name
// ("a.b.c" -> "c"; "a.b." -> "" ; "a" -> "a")
std::string nameNH(const std::string& name) {
  size_t pos = name.rfind(".");
  return (pos == std::string::npos) ?
    name : name.substr(pos + 1);
}

// Single port requirement
typedef boost::variant<boost::blank, Port::ConstPtr, size_t> PortReqVar;

/**
 * collect port token requirements from AST
 */
class PortReqVisitor {
public:
  typedef PortReqVar result_type;
  
  // return collected port requirements
  const PortReqMap& getResult() const
  { return portReq; }

  PortReqVar apply(const ASTNode::Ptr& n)
  { return apply_visitor(*this, n); }

  // see ASTNodeVisitor
  PortReqVar operator()(const ASTNodePortTokens& n)
  { return n.port(); }

  // see ASTNodeVisitor
  PortReqVar operator()(const ASTNodeLiteral& n)
  { return CoSupport::String::strAs<size_t>(n.value().get()); }

  // see ASTNodeVisitor
  PortReqVar operator()(const ASTNodeBinOp& n) {
    analyzeBinOpNode(
        apply(n.leftNode()),
        apply(n.rightNode()));
    return boost::blank();
  }

  // see ASTNodeVisitor
  PortReqVar operator()(const ASTNodeUnOp& n) {
    apply(n.childNode());
    return boost::blank();
  }

  // see ASTNodeVisitor
  PortReqVar operator()(const ASTNodeComm& n) {
    analyzeCommNode(
        n.port(),
        apply(n.childNode()));
    return boost::blank();
  }

  template<_OpBinT op>
  PortReqVar operator()(const ASTNodeBinOpXXX<op> &obj)
  { return (*this)(static_cast<const SGX::ASTNodeBinOp&>(obj)); }

  template<class T>
  PortReqVar operator()(const T& t)
  { return boost::blank(); }

private:
  // collected port requirements
  PortReqMap portReq;

  // internal ASTNode evaluation function
  void analyzeBinOpNode(PortReqVar l, PortReqVar r) {
    Port::ConstPtr* port = boost::get<Port::ConstPtr>(&l);
    if(port) {
      size_t* req = boost::get<size_t>(&r);
      assert(req);
      portReq[*port].second = *req;
    }
  }

  // internal ASTNode evaluation function
  void analyzeCommNode(Port::ConstPtr port, PortReqVar c) {
    assert(port);
    size_t* req = boost::get<size_t>(&c);
    assert(req);
    portReq[port].first = *req;
  }
};

smoc_graph_synth::smoc_graph_synth(ProblemGraph::ConstRef pg) :
  smoc_graph_base(nameNH(pg.name().get()).c_str(), init, false),
  pg(pg),
  curNode(nodeInfos.end())
{
  // register process and graph
  idPool.regObj(this, pg.owner()->id(), 0);
  idPool.regObj(this, pg.id(), 1);
  
  generateFSM();
}

void smoc_graph_synth::cachePhase(PortRefList::ConstRef ports, Phase& phase) {
  for(PortRefList::const_iterator pIter = ports.begin();
      pIter != ports.end();
      ++pIter)
  {
    Port::ConstPtr port = pIter->instance();
    smoc_sysc_port* rp =
      dynamic_cast<smoc_sysc_port*>(NGXCache::getInstance().get(*port));
    assert(rp);
    phase[rp] = pIter->attrValueAsSizeT("count").get();
  }
}

void smoc_graph_synth::cachePhases(Actor::ConstPtr actor, Phases& phases) {
  assert(actor->firingFSM());
  Classification::ConstPtr cl = actor->firingFSM()->classification();
  assert(cl);
  
  SDF::ConstPtr sdf = objAs<SDF>(cl);
  if(sdf) {
    phases.push_back(Phase());
    cachePhase(sdf->portTokens(), phases.back());
  }
  CSDF::ConstPtr csdf = objAs<CSDF>(cl);
  if(csdf) {
    CSDFPhaseList::ConstRef pList = csdf->phases();
    for(CSDFPhaseList::const_iterator pIter = pList.begin();
        pIter != pList.end();
        ++pIter)
    {
      phases.push_back(Phase());
      cachePhase(pIter->portTokens(), phases.back());
    }
  }
}

smoc_graph_synth::EVariant smoc_graph_synth::portGuard(
    PortReqMap::const_iterator i, PortReqMap::const_iterator e)
{
  assert(i != e);
  smoc_sysc_port *p = NGXCache::getInstance().getCompiledPort(i->first);
  assert(p);
  if (dynamic_cast<smoc_port_in_base_if *>(p->get_interface()) != NULL) {
    EPortInGuard pg =
      Expr::PortTokens<smoc_port_in_base_if>::type(*p) >= i->second.second;
    if (++i != e)
      return pg && portGuard(i, e);
    else
      return pg;
  } else {
    assert(dynamic_cast<smoc_chan_out_base_if *>(p->get_interface()) != NULL);
    EPortOutGuard pg =
      Expr::PortTokens<smoc_chan_out_base_if>::type(*p) >= i->second.second;
    if (++i != e)
      return pg && portGuard(i, e);
    else
      return pg;
  }
}

smoc_graph_synth::EVariant smoc_graph_synth::portGuard(
    const PortReqMap& pr)
{
  assert(!pr.empty());
  return portGuard(pr.begin(), pr.end());
}

void smoc_graph_synth::prepareActorFiring() {
  
  ActorFiring::ConstPtr af = objAs<ActorFiring>(ca);
  assert(af);
  assert(arm[af]);
  
  Actor::ConstPtr actor = af->actor();
  assert(actor);

  // get SystemC object
  smoc_root_node* rn =
    dynamic_cast<smoc_root_node*>(NGXCache::getInstance().get(*actor));
  assert(rn);

  // context switch to node
  curNode = nodeInfos.find(rn);

  if(curNode == nodeInfos.end()) {
    // actor was never activated until now
    curNode = CoSupport::DataTypes::pac_insert(nodeInfos, rn);
    NodeInfo& ni = curNode->second;
    cachePhases(actor, ni.phases);
    ni.count = 0;
    rn->addCurOutTransitions(ni.trans);
  }

  NodeInfo& ni = curNode->second;
  //std::cerr << " (phase: " << ni.count << ")" << std::endl;

  // exchange current EventWaiter
  transList.set(&ni.trans);

  // initialize limit token id map
  chanInLimit.clear();
  for(Phase::const_iterator i = ni.phases[ni.count].begin();
      i != ni.phases[ni.count].end();
      ++i)
  {
    if(i->first->isInput()) {
      // lookup chached interface
      ChanInMap::const_iterator j = chanInMap.find(i->first);

      if(j == chanInMap.end()) {
        // cross cast to obtain interface base class
        smoc_port_in_base_if* in =
          dynamic_cast<smoc_port_in_base_if*>(i->first->get_interface());
        assert(in);
        j = CoSupport::DataTypes::pac_insert(chanInMap, i->first, in);
      }

      // calculate limit id for channel
      chanInLimit[j->second] = j->second->inTokenId() + i->second;
    }
  }
  
  ni.count = (ni.count + 1) % ni.phases.size();
    
  --arm[af];
}

void smoc_graph_synth::prepareCompoundAction() {

  CompoundAction::ConstPtr cpa = objAs<CompoundAction>(ca);
  assert(cpa);
  assert(arm[cpa]);

  if(cpa->actions().begin() == cpa->actions().end()) {
    // empty compound action -> skip iterations...
    arm[cpa] = 0;
    return;
  }

  CompoundActionIterMap::iterator i = cpaim.find(cpa);
  if(i == cpaim.end()) {
    i = CoSupport::DataTypes::pac_insert(cpaim, cpa, cpa->actions().begin());
  }

  if(i->second == cpa->actions().end()) {
    if(--arm[cpa]) {
      i->second = cpa->actions().begin();
      setCurrentAction(i->second->toPtr());
    }
    else {
      // cleanup own structures
      cpaim.erase(i);
    }
  }
  else {
    setCurrentAction(i->second->toPtr());
    ++i->second;
  }
}

void smoc_graph_synth::prepareOtherAction() {
  assert(!"Unsupported action!");
}

bool smoc_graph_synth::isActorFiring() const {
  return objAs<ActorFiring>(ca);
}

bool smoc_graph_synth::isCompoundAction() const {
  return objAs<CompoundAction>(ca);
}

void smoc_graph_synth::setCurrentAction(Action::ConstPtr a) {
  ca = a;
  if(ca)
    arm[ca] =
      ca->repeat().isDefined() ? ca->repeat().get() : 1;
}

void smoc_graph_synth::parentAction() {
  ca = ca->parent();
}

bool smoc_graph_synth::haveAction() const {
  return ca;
}

bool smoc_graph_synth::iterationsLeft() const {
  ActionRepeatMap::const_iterator i = arm.find(ca);
  assert(i != arm.end());
  return i->second;
}

bool smoc_graph_synth::activationComplete() const {
  for(ChanInLimit::const_iterator i = chanInLimit.begin();
      i != chanInLimit.end();
      ++i)
  {
    // don't use "<" because of wrap-around logic
    if(i->first->inTokenId() != i->second)
      return false;
  }
  return true;
}

void smoc_graph_synth::executeTransitionList() {
  assert(curNode != nodeInfos.end());
  smoc_root_node* rn = curNode->first;
  NodeInfo& ni = curNode->second;

  RuntimeTransition &trans = ni.trans.getEventTrigger();
  Expr::Detail::ActivationStatus status = trans.getStatus();

  switch(status.toSymbol()) {
    case Expr::Detail::_DISABLED:
      assert(&trans.getActor() == rn);
      ni.trans.remove(trans);
      break;
    case Expr::Detail::_ENABLED:
      assert(&trans.getActor() == rn);
#ifdef SYSTEMOC_DEBUG
      std::cerr << "<node name=\"" << rn->name() << "\">" << std::endl;
#endif
      // remove transitions from list
      rn->delCurOutTransitions(ni.trans);
      // execute transition
      trans.execute();
      // add transitions to list
      rn->addCurOutTransitions(ni.trans);
#ifdef SYSTEMOC_DEBUG
      std::cerr << "</node>" << std::endl;
#endif
      break;
    default:
      assert(0);
  }
}

void smoc_graph_synth::generateFSM() {

  FiringFSM::ConstPtr fsm = pg.firingFSM();
  if(!fsm) {
    std::cerr << "synthesized graph has no FSM" << std::endl;
    return;
  }

  // create state lookup table
  typedef std::map<FiringState::ConstPtr, smoc_firing_state::Ptr> StateMap;
  StateMap sm;

  // initialize state lut
  assert(fsm->startState());
  sm[fsm->startState()] = init.toPtr();

  FiringStateList::ConstRef states = fsm->states();
  for(FiringStateList::const_iterator sIter = states.begin();
      sIter != states.end();
      ++sIter)
  {
    FiringState::ConstPtr sSrc = sIter.operator->();
    smoc_transition_list tl;

    FiringTransitionList::ConstRef ts = sSrc->outTransitions();
    for(FiringTransitionList::const_iterator tIter = ts.begin();
        tIter != ts.end();
        ++tIter)
    {
      FiringTransition::ConstRef t = *tIter;
      FiringState::ConstPtr sDst = t.dstState();

      // construct transition activation pattern
      PortReqVisitor vPR;
      vPR.apply(t.activationPattern());

      smoc_activation_pattern apClusterIO = portGuard(vPR.getResult());

#ifdef SYSTEMOC_DEBUG
      smoc_event_and_list al;
      Expr::evalTo<Expr::Sensitivity>(ap.guard, al);
      al.dump(std::cerr); std::cerr << std::endl;
#endif

      if(!sm[sDst]) sm[sDst] = smoc_firing_state().toPtr();
      smoc_firing_state setup, run, check;

      tl |=
           // cluster i/o-port requirements (no commit)
           apClusterIO
           // prepare action for execution
        >> CALL(smoc_graph_synth::setCurrentAction)(t.action())
        >> setup;

      setup =
           (GUARD(smoc_graph_synth::haveAction) &&
            GUARD(smoc_graph_synth::iterationsLeft) &&
            GUARD(smoc_graph_synth::isActorFiring))
        >> CALL(smoc_graph_synth::prepareActorFiring)
        >> run
      |    (GUARD(smoc_graph_synth::haveAction) &&
            GUARD(smoc_graph_synth::iterationsLeft) &&
            GUARD(smoc_graph_synth::isCompoundAction))
        >> CALL(smoc_graph_synth::prepareCompoundAction)
        >> setup
      |    (GUARD(smoc_graph_synth::haveAction) &&
            GUARD(smoc_graph_synth::iterationsLeft) &&
            !GUARD(smoc_graph_synth::isActorFiring) &&
            !GUARD(smoc_graph_synth::isCompoundAction))
        >> CALL(smoc_graph_synth::prepareOtherAction)
        >> setup
      |    (GUARD(smoc_graph_synth::haveAction) &&
            !GUARD(smoc_graph_synth::iterationsLeft))
        >> CALL(smoc_graph_synth::parentAction)
        >> setup
           // no other executable action in schedule
      |    !GUARD(smoc_graph_synth::haveAction)
        >> *sm[sDst];

      run =
           // prepared transition list ready
           Expr::till(transList)
           // execute transition
        >> CALL(smoc_graph_synth::executeTransitionList)
        >> check;

      check =
           // actor has reserved tokens / space left
           !GUARD(smoc_graph_synth::activationComplete)
        >> run
           // actor has no reserved tokens / space left
      |    GUARD(smoc_graph_synth::activationComplete)
        >> setup;
    }

    // assign transition list to state
    if(!sm[sSrc]) sm[sSrc] = smoc_firing_state().toPtr();
    *sm[sSrc] = tl;
  }

//// delete temporary firing states
//for(StateMap::const_iterator i = sm.begin(); i != sm.end(); ++i) {
//  if(i->second != &init)
//    delete i->second;
//}
}

} // namespace SysteMoC
