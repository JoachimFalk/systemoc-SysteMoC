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

#include <typeinfo>

#include <systemoc/smoc_config.h>

#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/smoc_root_node.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <systemoc/smoc_pggen.hpp>
#include <systemoc/smoc_ngx_sync.hpp>
#include <systemoc/hscd_tdsim_TraceLog.hpp>

using namespace SysteMoC::NGXSync;

smoc_root_node::smoc_root_node(sc_module_name name, smoc_hierarchical_state &s, bool regObj)
  : sc_module(name),
#if defined(SYSTEMOC_ENABLE_DEBUG)
//  _finalizeCalled(false),
#endif
    initialState(s),
    _non_strict(false)
#ifdef SYSTEMOC_ENABLE_VPC
    ,diiEvent(new smoc_ref_event())
//# if defined(SYSTEMOC_ENABLE_DEBUG)
//    vpc_event_lat(NULL),
//# endif
#endif // SYSTEMOC_ENABLE_VPC
//  _guard(NULL)
{
#ifdef SYSTEMOC_ENABLE_VPC
  commstate = new RuntimeState();
  commstate->getTransitions().push_back(
      RuntimeTransition(
        this,
        Expr::till(*diiEvent),
        smoc_func_diverge(this, &smoc_root_node::_communicate)));
#endif // SYSTEMOC_ENABLE_VPC

  local_constr_args.insert(
      local_constr_args.end(),
      global_constr_args.begin(),
      global_constr_args.end());
  global_constr_args.clear();
  current_actor = this;
  /*
  while(!smoc_root_node::global_constr_args.empty()){
    local_arg_vector.push_back(smoc_root_node::global_arg_stack.top());
    smoc_root_node::global_arg_stack.pop();
  }*/
  if(regObj) idPool.regObj(this);
}
 
smoc_root_node *smoc_root_node::current_actor = NULL;
std::vector<std::pair<std::string, std::string> >smoc_root_node::global_constr_args; 
 
#ifdef SYSTEMOC_ENABLE_VPC
RuntimeState* smoc_root_node::_communicate() {
  assert(diiEvent != NULL && *diiEvent); // && vpc_event_lat != NULL
  return nextState;
}
#endif // SYSTEMOC_ENABLE_VPC

void smoc_root_node::finalise() {
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_root_node::finalise() begin, name == " << this->name() << std::endl;
#endif
  // Preallocate ID
  //smoc_modes::PGWriter::getId(this);
 
  getFiringFSM()->finalise(this, initialState.getImpl());
  currentState = getFiringFSM()->getInitialState();

  //getFiringFSM()->dumpDot(currentState);

  //std::cerr << "smoc_root_node::finalise() name == " << this->name() << std::endl
  //          << "  FiringFSM: " << currentState->getFiringFSM()
  //          << "; #leafStates: " << currentState->getFiringFSM()->getLeafStates().size()
  //          << std::endl;

  smoc_sysc_port_list ports = getPorts();
  
  for (smoc_sysc_port_list::iterator iter = ports.begin();
       iter != ports.end();
       ++iter) {
    assert(*iter != NULL);
    (*iter)->finalise(this);
  }
  
  //check for non strict transitions
  const RuntimeStateSet& states = getFiringFSM()->getStates(); 

  for(RuntimeStateSet::const_iterator sIter = states.begin(); 
      sIter != states.end();
      ++sIter)
  {
    const RuntimeTransitionList& tl = (*sIter)->getTransitions();
    
    for(RuntimeTransitionList::const_iterator tIter = tl.begin();
        tIter != tl.end();
        ++tIter)
    {
      if(boost::get<smoc_sr_func_pair>(&tIter->getAction())) {
#ifdef SYSTEMOC_DEBUG
          cout << "found non strict SR block: " << this->name() << endl;
#endif
          _non_strict = true;
      }
    }
  }
#ifdef SYSTEMOC_DEBUG
  std::cerr << "smoc_root_node::finalise() end, name == " << this->name() << std::endl;
#endif
}

smoc_sysc_port_list smoc_root_node::getPorts() const {
  smoc_sysc_port_list ret;
  
  for( 
#if SYSTEMC_VERSION < 20050714
    sc_pvector<sc_object*>::const_iterator iter =
#else
    std::vector<sc_object*>::const_iterator iter =
#endif
      get_child_objects().begin();
    iter != get_child_objects().end(); ++iter)
  {
    if(smoc_sysc_port* p = dynamic_cast<smoc_sysc_port*>(*iter))
      ret.push_back(p);
  }
  return ret;
}

RuntimeStateList smoc_root_node::getStates() const { 
  RuntimeStateList ret;

  for( 
#if SYSTEMC_VERSION < 20050714
    sc_pvector<sc_object*>::const_iterator iter =
#else
    std::vector<sc_object*>::const_iterator iter =
#endif
      get_child_objects().begin();
    iter != get_child_objects().end(); ++iter)
  {
    if(RuntimeState* s = dynamic_cast<RuntimeState*>(*iter))
      ret.push_back(s);
  }
  return ret;
}

void smoc_root_node::pgAssemble( smoc_modes::PGWriter &pgw, const smoc_root_node *n ) const
  {}

void smoc_root_node::assemble( smoc_modes::PGWriter &pgw ) const {
  const smoc_sysc_port_list ps = getPorts();
  
  if ( !ps.empty() ) {
    pgw << "<process name=\"" << name() << "\" "
                    "type=\"actor\" "
                    "id=\"" << idPool.printId(this) << "\">" << std::endl;
    {
      pgw.indentUp();
      //**********************************PORTS************************************
      for ( smoc_sysc_port_list::const_iterator iter = ps.begin();
            iter != ps.end();
            ++iter )
        pgw << "<port name=\"" << (*iter)->name() << "\" "
                     "type=\"" << ((*iter)->isInput() ? "in" : "out") << "\" "
                     "id=\"" << idPool.printId(*iter) << "\"/>" << std::endl;
      //**************************FSM-STATES  && ACTOR-TAG*************************
      assembleActor(pgw);
      //***************************CONTAINED PROBLEMGRAPH**************************
      pgAssemble(pgw, this);
      pgw.indentDown();
    }
    pgw << "</process>" << std::endl;
  } else {
    pgAssemble(pgw, this);
  }
}

/*FIXME: function constructs dump of different tag-levels. necessary, because
  smoc_graph (inherits from smoc_root_node) must not construct this output.
  So it reimplements this virtual function.*/
void smoc_root_node::assembleActor(smoc_modes::PGWriter &pgw ) const {

  //*********************************FSM-STATES********************************
  // FIXME: this assumes that SystemC will iterate objects sorted by construction
  // order (but seems to be sorted by name, wich is also correct _UNLESS_ user
  // specifies own name for a firing state!!!)
  const RuntimeStateList& states = getStates();
  
  for(RuntimeStateList::const_iterator iter = states.begin();
      iter != states.end();
      ++iter)
  {
#ifdef SYSTEMOC_ENABLE_VPC
    // do not dump comm state
    if(*iter == getCommState())
      continue;
#endif
    pgw << "<stateDeclaration state=\"" << idPool.printId(*iter)
        << "\" name=\"" << (*iter)->name()
        << "\"/>" << std::endl;
  }
  
  //*******************************ACTOR CLASS*********************************
  pgw << "<actor actorClass=\"" << typeid(*this).name() << "\">" << std::endl;
  pgw.indentUp();
    //***************************CONSTRUCTORPARAMETERS***************************
    for (unsigned int i = 0; i < local_constr_args.size(); i++) {
      std::pair<std::string, std::string> parameterpair = local_constr_args[i];
      pgw << "<constructorParameter type=\"" << parameterpair.first
          << "\" value=\"" << parameterpair.second << "\"/>"
          << std::endl;
    }
    //************************************FSM************************************
    assembleFSM(pgw);
  pgw.indentDown();
  pgw << "</actor>" << std::endl;
}

namespace {
  using namespace SysteMoC::ActivationPattern;

  class ASTXMLDumperVisitor {
  public:
    typedef void result_type;
  private:
    smoc_modes::PGWriter &pgw;
  protected:
    void openNodeTag(const ASTNode &astNode) {
      pgw << "<" << astNode.getNodeType() 
                 << " valueType=\"" << astNode.getValueType() << "\"";
    }
    void closeNodeTag(const ASTNode &astNode) {
      pgw << "</" << astNode.getNodeType() << ">" << std::endl;
    }
    void dumpASTUnNode(const ASTInternalUnNode &astNode) {
      pgw.indentUp();
      pgw << "<ChildNode>" << std::endl;
      {
        pgw.indentUp();
        apply_visitor(*this, astNode.getChildNode());
        pgw.indentDown();
      }
      pgw << "</ChildNode>" << std::endl;
      pgw.indentDown();
    }
    void dumpASTBinNode(const ASTInternalBinNode &astNode) {
      pgw.indentUp();
      pgw << "<lhs>" << std::endl;
      {
        pgw.indentUp();
        apply_visitor(*this, astNode.getLeftNode());
        pgw.indentDown();
      }
      pgw << "</lhs>" << std::endl;
      pgw << "<rhs>" << std::endl;
      {
        pgw.indentUp();
        apply_visitor(*this, astNode.getRightNode());
        pgw.indentDown();
      }
      pgw << "</rhs>" << std::endl;
      pgw.indentDown();
    }
  public:
    ASTXMLDumperVisitor(smoc_modes::PGWriter &pgw)
      : pgw(pgw) {}

    result_type operator ()(ASTNodeVar &astNode) {
      openNodeTag(astNode);
      pgw << " name=\"" << astNode.getName() << "\">";
//    pgw << " addr=\"0x" << std::hex << reinterpret_cast<unsigned long>(astNode.getAddr()) << "\">";
      closeNodeTag(astNode);
    }
    result_type operator ()(ASTNodeLiteral &astNode) {
      openNodeTag(astNode);
      pgw << " value=\"" << astNode.getValue() << "\">";
      closeNodeTag(astNode);
    }
    result_type operator ()(ASTNodeProc &astNode) {
      assert(!"Unimplemented");
    }
    result_type operator ()(ASTNodeMemProc &astNode) {
      assert(!"Unimplemented");
    }
    result_type operator ()(ASTNodeMemGuard &astNode) {
      openNodeTag(astNode);
      pgw << " name=\"" << astNode.getName() << "\">" << std::endl;
      pgw.indentUp();
      for(ParamInfoList::const_iterator i = astNode.getParams().begin();
          i != astNode.getParams().end(); ++i)
      {
        pgw << "<parameter name=\"" << i->name << "\""
            <<           " valueType=\"" << i->type << "\""
            <<           " value=\"" << i->value << "\"/>" << std::endl;
      }
      pgw.indentDown();
//    pgw << " addrObj=\"0x" << std::hex << reinterpret_cast<unsigned long>(astNode.getAddrObj()) << std::dec << "\"";
//    pgw << " addrFun=\"0x" << std::hex << reinterpret_cast<unsigned long>(astNode.getAddrFun()) << std::dec << "\">";
      closeNodeTag(astNode);
    }
    result_type operator ()(ASTNodeToken &astNode) {
      assert(astNode.getPortId().getPortPtr() != NULL);
      const smoc_sysc_port *port = dynamic_cast<const smoc_sysc_port *>(astNode.getPortId().getPortPtr());
      assert(port != NULL);
      openNodeTag(astNode);
      pgw << " portid=\"" << idPool.printId(port) << "\"";
      pgw << " pos=\"" << astNode.getPos() << "\">";
      closeNodeTag(astNode);
    }
    result_type operator ()(ASTNodePortTokens &astNode) {
      assert(astNode.getPortId().getPortPtr() != NULL);
      const smoc_sysc_port *port = dynamic_cast<const smoc_sysc_port *>(astNode.getPortId().getPortPtr());
      assert(port != NULL);
      openNodeTag(astNode);
      pgw << " portid=\"" << idPool.printId(port) << "\">";
      closeNodeTag(astNode);
    }
    result_type operator ()(ASTNodeSMOCEvent &astNode) {
      openNodeTag(astNode);
      //assert(!"Unimplemented");
      pgw << ">";
      closeNodeTag(astNode);
    }
    result_type operator ()(ASTNodePortIteration &astNode) {
      //assert(!"Unimplemented");
    }
    result_type operator ()(ASTNodeBinOp &astNode) {
      openNodeTag(astNode);
      pgw << " opType=\"" << astNode.getOpType() << "\">" << std::endl;
      dumpASTBinNode(astNode);
      closeNodeTag(astNode);
    }
    result_type operator ()(ASTNodeUnOp &astNode) {
      openNodeTag(astNode);
      pgw << " opType=\"" << astNode.getOpType() << "\">" << std::endl;
      dumpASTUnNode(astNode);
      closeNodeTag(astNode);
    }
    result_type operator ()(ASTNodeComm &astNode) {
      assert(astNode.getPortId().getPortPtr() != NULL);
      const smoc_sysc_port *port = dynamic_cast<const smoc_sysc_port *>(astNode.getPortId().getPortPtr());
      assert(port != NULL);
      openNodeTag(astNode);
      pgw << " portid=\"" << idPool.printId(port) << "\">" << std::endl;
      dumpASTUnNode(astNode);
      closeNodeTag(astNode);
    }
  };
}

void smoc_root_node::assembleFSM( smoc_modes::PGWriter &pgw ) const {
  
  const RuntimeStateSet& states = getFiringFSM()->getStates();
  
  pgw << "<fsm startstate=\"" << idPool.printId(getFiringFSM()->getInitialState())
      << "\">" << std::endl;
  pgw.indentUp();
  {
    //******************************FSMSTATES************************************ 
    for(RuntimeStateSet::const_iterator sIter = states.begin(); 
        sIter != states.end(); 
        ++sIter)
    {
      pgw << "<state id=\"" << idPool.printId(*sIter)
          << "\" name=\"" << (*sIter)->name()
          << "\">" << std::endl;
      pgw.indentUp();
      {
        //**************TRANTIONS********************
        const RuntimeTransitionList& tl = (*sIter)->getTransitions();
    
        for(RuntimeTransitionList::const_iterator tIter = tl.begin();
            tIter != tl.end();
            ++tIter)
        {
          const RuntimeState* dest = tIter->getDestState();
          
          pgw << "<transition nextstate=\"" << idPool.printId(dest) << "\">" << std::endl;
          pgw.indentUp();
          {
            //**************ACTION********************
            const smoc_func_call_list* fcl;
              
            if((fcl = boost::get<smoc_func_call_list>(&tIter->getAction())) &&
                  !fcl->empty()) {
               
              bool single = (fcl->end() == ++fcl->begin());
              if(!single) {
                pgw << "<action>" << std::endl;
                pgw.indentUp();
                pgw << "<compoundAction>" << std::endl;
                pgw.indentUp();
              }

              for(smoc_func_call_list::const_iterator fc = fcl->begin();
                  fc != fcl->end(); ++fc)
              {
                pgw << "<action>" << std::endl;
                pgw.indentUp();
                pgw << "<function name=\"" << fc->getFuncName() << "\">" << std::endl;
                pgw.indentUp();
                {
                  //**************PARAMETER********************
                  for(ParamInfoList::const_iterator iter = fc->getParams().begin();
                      iter != fc->getParams().end(); ++iter)
                  {
                    pgw << "<parameter name=\"" << iter->name << "\""
                        <<           " valueType=\"" << iter->type << "\""
                        <<           " value=\""
                        << CoSupport::Streams::TranslationMap::XMLAttr()
                        << iter->value
                        << CoSupport::Streams::TranslationMap::None()
                        << "\"/>" << std::endl;
                  }
                  //**************/PARAMETER********************
                }
                pgw.indentDown();
                pgw << "</function>" << std::endl;
                pgw.indentDown();
                pgw << "</action>" << std::endl;
              }

              if(!single) {
                pgw.indentDown();
                pgw << "</compoundAction>" << std::endl;
                pgw.indentDown();
                pgw << "</action>" << std::endl;
              }
            }
            else {
              pgw << "<action />" << std::endl;
            }
            //**************/ACTION********************
          }
          ASTXMLDumperVisitor astDumper(pgw);
          apply_visitor(astDumper, Expr::evalTo<Expr::AST>(tIter->getExpr()));

          pgw.indentDown();
          pgw << "</transition>" << std::endl;
        }
        //***************/TRANTIONS*****************
      }
      pgw.indentDown();
      pgw << "</state>" << std::endl;
    }
    //*********************************/FSMSTATES*************************************
  }
  pgw.indentDown();
  pgw << "</fsm>" << std::endl;
}
 
std::ostream &smoc_root_node::dumpActor(std::ostream &o) {
  o << "actor: " << this->name() << std::endl;
  smoc_sysc_port_list ps = getPorts();
  o << "  ports:" << std::endl;
  for ( smoc_sysc_port_list::const_iterator iter = ps.begin();
        iter != ps.end();
        ++iter ) {
    o << "  " << *iter << std::endl;
  }
  return o;
}

bool smoc_root_node::inCommState() const{
#ifdef SYSTEMOC_ENABLE_VPC
    return currentState == getCommState();
#else // SYSTEMOC_ENABLE_VPC
    return false;
#endif // SYSTEMOC_ENABLE_VPC
}

bool smoc_root_node::isNonStrict() const{
  return _non_strict;
}

void Expr::Detail::registerParam(const ArgInfo &argInfo) {
  smoc_root_node::global_constr_args.push_back(argInfo);
}

void Expr::Detail::registerParamOnCurrentActor(const ArgInfo &argInfo) {
  smoc_root_node::current_actor->local_constr_args.push_back(argInfo);
}

void smoc_root_node::addCurOutTransitions(smoc_transition_ready_list& ol) const {
  assert(currentState);
  for(RuntimeTransitionList::iterator tIter =
        currentState->getTransitions().begin();
      tIter != currentState->getTransitions().end();
      ++tIter)
  {
    ol |= *tIter;
  }
}

void smoc_root_node::delCurOutTransitions(smoc_transition_ready_list& ol) const {
  assert(currentState);
  for(RuntimeTransitionList::iterator tIter =
        currentState->getTransitions().begin();
      tIter != currentState->getTransitions().end();
      ++tIter)
  {
    ol.remove(*tIter);
  }
}

smoc_root_node::~smoc_root_node() {
  idPool.unregObj(this);
  delete commstate;
}
