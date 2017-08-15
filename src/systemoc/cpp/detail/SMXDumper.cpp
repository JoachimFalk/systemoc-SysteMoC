//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_SGX

#include <map>
#include <utility>
#include <memory>

#include <boost/scoped_ptr.hpp>

#include <smoc/smoc_expr.hpp>

#include <sgx.hpp>
#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/String/Concat.hpp>

#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/DumpingInterfaces.hpp>
#include <smoc/detail/NamedIdedObj.hpp>

#include "apply_visitor.hpp"
#include "SimulationContext.hpp"

#include <smoc/detail/Node.hpp>
#include <smoc/detail/Chan.hpp>
#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/detail/smoc_firing_rules_impl.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_multiplex_fifo.hpp>
#include <systemoc/smoc_multireader_fifo.hpp>

//#define SYSTEMOC_DEBUG

namespace smoc { namespace Detail {

namespace SGX = SystemCoDesigner::SGX;

typedef std::map<sc_core::sc_port_base const *, SGX::Port::Ptr>  SCPortBase2Port;
typedef std::map<sc_core::sc_interface *, SGX::Port::Ptr>  SCInterface2Port;

using CoSupport::String::Concat;

SimulationContextSMXDumping::SimulationContextSMXDumping()
  : dumpPreSimSMXKeepGoing(false)
  , dumpSMXAST(true)
  , dumpPreSimSMXFile(nullptr)
  , dumpPostSimSMXFile(nullptr) {}

class ActionNGXVisitor {
public:
  typedef SGX::Action::Ptr result_type;
public:
  result_type operator()(smoc_func_call_list &f) const;
};

SGX::Action::Ptr ActionNGXVisitor::operator()(smoc_func_call_list &f) const {
  if (f.empty())
    return nullptr;
  
  bool single = (++f.begin() == f.end());
  SGX::CompoundAction top;
  
  for (smoc_func_call_list::iterator i = f.begin(); i != f.end(); ++i) {
    SGX::Function func;
    func.name() = i->getFuncName();
    func.cxxType() = i->getCxxType();
    
    for (ParamInfoList::const_iterator pIter = i->getParams().begin();
         pIter != i->getParams().end();
         ++pIter) {
      SGX::Parameter p(pIter->type, pIter->value);
      //p.name() = pIter->name;
      func.parameters().push_back(p);
    }
    if (single)
      return &func;
    else
      top.actions().push_back(func);
  }
  
  return &top;
}

class ExprNGXVisitor: public ExprVisitor<SGX::ASTNode> {
  typedef ExprVisitor<SGX::ASTNode> base_type;
  typedef ExprNGXVisitor            this_type;
protected:
  SCPortBase2Port &ports;
public:
  ExprNGXVisitor(SCPortBase2Port &ports)
    : ports(ports) {}

  result_type visitVar(const std::string &name, const std::string &type);
  result_type visitLiteral(const std::string &type, const std::string &value);
  result_type visitMemGuard(const std::string &name, const std::string& cxxType, const std::string &reType, const ParamInfoList &params);
  result_type visitEvent(const std::string &name);
  result_type visitPortTokens(smoc_sysc_port &p);
  result_type visitToken(smoc_sysc_port &p, size_t n);
  result_type visitComm(smoc_sysc_port &p, boost::function<result_type (base_type &)> e);
  result_type visitUnOp(OpUnT op, boost::function<result_type (base_type &)> e);
  result_type visitBinOp(OpBinT op, boost::function<result_type (base_type &)> a, boost::function<result_type (base_type &)> b);
};

ExprNGXVisitor::result_type ExprNGXVisitor::visitVar(const std::string &name, const std::string &type) {
  std::unique_ptr<SGX::ASTNodeVar> astNode(new SGX::ASTNodeVar);
  astNode->name() = name;
  astNode->valueType() = type;
  return astNode.release();
}

ExprNGXVisitor::result_type ExprNGXVisitor::visitLiteral(const std::string &type, const std::string &value) {
  std::unique_ptr<SGX::ASTNodeLiteral> astNode(new SGX::ASTNodeLiteral);
  astNode->valueType() = type;
  astNode->value() = value;
  return astNode.release();
}

ExprNGXVisitor::result_type ExprNGXVisitor::visitMemGuard(
    const std::string &name, const std::string& cxxType, const std::string &reType, const ParamInfoList &params) {
  std::unique_ptr<SGX::ASTNodeMemGuard> astNode(new SGX::ASTNodeMemGuard);
  astNode->name() = name;
  astNode->valueType() = reType;
  astNode->cxxType() = cxxType;
  
  SGX::ParameterList::Ref sgxParams = astNode->parameters();
  for (ParamInfoList::const_iterator pIter = params.begin();
       pIter != params.end();
       ++pIter) {
    SGX::Parameter p(pIter->type, pIter->value);
    sgxParams.push_back(p);
  }
  return astNode.release();
}

ExprNGXVisitor::result_type ExprNGXVisitor::visitEvent(const std::string &name) {
  std::unique_ptr<SGX::ASTNodeEvent> astNode(new SGX::ASTNodeEvent);
  astNode->name() = name;
  astNode->valueType() = typeid(bool).name();
  return astNode.release();
}

ExprNGXVisitor::result_type ExprNGXVisitor::visitPortTokens(smoc_sysc_port &p) {
  std::unique_ptr<SGX::ASTNodePortTokens> astNode(new SGX::ASTNodePortTokens);
  SCPortBase2Port::iterator iter = ports.find(&p);
  assert(iter != ports.end() && "WTF?!: Got port in activation pattern which is not from the same actor as the FSM?!");
  astNode->port() = iter->second;
  astNode->valueType() = typeid(size_t).name();
  return astNode.release();
}

ExprNGXVisitor::result_type ExprNGXVisitor::visitToken(smoc_sysc_port &p, size_t n) {
  std::unique_ptr<SGX::ASTNodeToken> astNode(new SGX::ASTNodeToken);
  SCPortBase2Port::iterator iter = ports.find(&p);
  assert(iter != ports.end() && "WTF?!: Got port in activation pattern which is not from the same actor as the FSM?!");
  astNode->port() = iter->second;
  astNode->pos() = n;
  return astNode.release();
}

ExprNGXVisitor::result_type ExprNGXVisitor::visitComm(smoc_sysc_port &p, boost::function<result_type (base_type &)> e) {
  std::unique_ptr<SGX::ASTNodeComm> astNode(new SGX::ASTNodeComm);
  SCPortBase2Port::iterator iter = ports.find(&p);
  assert(iter != ports.end() && "WTF?!: Got port in activation pattern which is not from the same actor as the FSM?!");
  astNode->port() = iter->second;
  std::unique_ptr<SGX::ASTNode> childNode(e(*this));
  astNode->childNode() = childNode->toPtr();
  return astNode.release();
}

ExprNGXVisitor::result_type ExprNGXVisitor::visitUnOp(
    OpUnT op,
    boost::function<result_type (base_type &)> e)
{
  std::unique_ptr<SGX::ASTNodeUnOp> astNode(new SGX::ASTNodeUnOp);
  astNode->opType() = op;
  std::unique_ptr<SGX::ASTNode> childNode(e(*this));
  astNode->childNode() = childNode->toPtr();
  return astNode.release();
}

ExprNGXVisitor::result_type ExprNGXVisitor::visitBinOp(
    OpBinT op,
    boost::function<result_type (base_type &)> a,
    boost::function<result_type (base_type &)> b)
{
  std::unique_ptr<SGX::ASTNodeBinOp> astNode(new SGX::ASTNodeBinOp);
  astNode->opType() = op;
  std::unique_ptr<SGX::ASTNode> leftNode(a(*this));
  astNode->leftNode() = leftNode->toPtr();
  std::unique_ptr<SGX::ASTNode> rightNode(b(*this));
  astNode->rightNode() = rightNode->toPtr();
  return astNode.release();
}

/*

ExprNGXVisitor::result_type ExprNGXVisitor::operator()(ASTNodePortIteration &a) {
  assert(!"Unimplemented");
  // FIXME PROBLEMATIC!!!
}

*/

struct SMXDumpCTX {
  SimulationContextSMXDumping *simCTX;

  SMXDumpCTX(SimulationContextSMXDumping *ctx)
    : simCTX(ctx) {}
};

template <class Visitor>
void recurse(Visitor &visitor, sc_core::sc_object &obj) {
#if SYSTEMC_VERSION < 20050714
  typedef sc_core::sc_pvector<sc_core::sc_object*> sc_object_list;
#else
  typedef std::vector<sc_core::sc_object*>         sc_object_list;
#endif
  {
    sc_core::sc_module *mod;
    for (sc_object_list::const_iterator iter = obj.get_child_objects().begin();
         iter != obj.get_child_objects().end();
         ++iter) {
      // Actors/Graphs first!
      if ((mod = dynamic_cast<sc_core::sc_module *>(*iter))) {
        // But ignore internal SysteMoC sc_modules, which should start with __smoc_!
        // -> first identify leaf name of the hierarchical SystemC name, i.e., if name == "a.b.c" then leaf name == "c"
        std::string            name = mod->name();
        std::string::size_type pos  = name.rfind('.');
        if (pos != std::string::npos)
          pos++;
        else
          pos = 0;
        // Now name.substr(pos, ...) is the leaf name
        if (name.substr(pos, sizeof("__smoc_")-1) != "__smoc_")
          apply_visitor(visitor, *static_cast<sc_core::sc_module *>(*iter));
      }
    }
  }
  {
    Chan *chan;
    for (sc_object_list::const_iterator iter = obj.get_child_objects().begin();
         iter != obj.get_child_objects().end();
         ++iter) {
      // Channels next!
      if ((chan = dynamic_cast<Chan *>(*iter)))
        apply_visitor(visitor, *chan);
    }
  }
  {
    sc_core::sc_port_base *port;
    for (sc_object_list::const_iterator iter = obj.get_child_objects().begin();
         iter != obj.get_child_objects().end();
         ++iter) {
      // Ports last!
      if ((port = dynamic_cast<sc_core::sc_port_base *>(*iter)))
        apply_visitor(visitor, *port);
    }
  }
}

class GraphSubVisitor;

struct ExpectedPortConnections {
  // map from channel entry/outlet to inner port
  SCInterface2Port   unclassifiedPorts;
  // map from outer port to inner port
  SCPortBase2Port    expectedOuterPorts;
  // map from channel entry/outlet to inner port
  SCInterface2Port   expectedChannelConnections;

  ~ExpectedPortConnections() {
    assert(expectedOuterPorts.empty());
  /*Disable this till we have covered all channel types
    assert(expectedChannelConnections.empty());*/
    for (SCInterface2Port::const_iterator iter = expectedChannelConnections.begin();
         iter != expectedChannelConnections.end();
         ++iter) {
      PortOutBaseIf *entry;
      PortInBaseIf  *outlet;
      
      if ((entry = dynamic_cast<PortOutBaseIf *>(iter->first))) {
        std::cerr << "Unhandled entry type " << typeid(*entry).name()
                  << " => dangling port " << iter->second->name() << std::endl;
      } else if ((outlet = dynamic_cast<PortInBaseIf *>(iter->first))) {
        std::cerr << "Unhandled outlet type " << typeid(*outlet).name()
                  << " => dangling port " << iter->second->name() << std::endl;
      } else {
        //FIXME: RTX hack dynamic_cast<...>(...) problem strikes again!!!
        //assert(!"WTF?! Neither PortOutBaseIf nor PortInBaseIf!");
        std::cerr << "Unhandled entry/outlet type " << typeid(iter->first).name()
                  << " => dangling port " << iter->second->name() << std::endl;
      }
    }
  }
};

class ProcessSubVisitor: public ExpectedPortConnections {
public:
  typedef void result_type;
public:
  SMXDumpCTX              &ctx;
  // one hierarchy up
  ExpectedPortConnections &epc;
  SGX::Process            &proc;
  SCPortBase2Port          ports;
public:
  SGX::Process &getProcess()
    { return proc; }
public:
  ProcessSubVisitor(SMXDumpCTX &ctx, ExpectedPortConnections &epc, SGX::Process &proc)
    : ctx(ctx), epc(epc), proc(proc) {}

  void operator ()(smoc_sysc_port &obj);

  void operator ()(sc_core::sc_port_base &obj);

  void operator ()(sc_core::sc_object &obj)
    { /* ignore */ }

};

class GraphSubVisitor: public ProcessSubVisitor {
public:
  typedef void result_type;
public:
  SGX::ProblemGraph &pg;
public:
  SGX::RefinedProcess &getRefinedProcess()
    { return static_cast<SGX::RefinedProcess &>(proc); }
public:
  GraphSubVisitor(SMXDumpCTX &ctx, ExpectedPortConnections &epc, SGX::RefinedProcess &proc, SGX::ProblemGraph &pg)
    : ProcessSubVisitor(ctx, epc, proc), pg(pg) {}

  void operator ()(GraphBase &obj);

  void operator ()(sc_core::sc_module &obj);

  void operator ()(smoc_actor &obj);

  void operator ()(smoc_fifo_chan_base &obj);

  void operator ()(smoc_multireader_fifo_chan_base &obj);

  void operator ()(smoc_multiplex_fifo_chan_base &obj);

  void operator ()(smoc_reset_chan &obj);

  ~GraphSubVisitor();

  using ProcessSubVisitor::operator();
};

class ActorSubVisitor: public ProcessSubVisitor {
public:
  typedef void result_type;
protected:
  SGX::Actor &getActor()
    { return static_cast<SGX::Actor &>(proc); }
public:
  ActorSubVisitor(SMXDumpCTX &ctx, ExpectedPortConnections &epc, SGX::Actor &actor)
    : ProcessSubVisitor(ctx, epc, actor) {}

  using ProcessSubVisitor::operator();
};

class DumpPort: public NamedIdedObjAccess {
public:
  typedef void result_type;
protected:
  ProcessSubVisitor &psv;
public:
  DumpPort(ProcessSubVisitor &psv)
    : psv(psv) {}

  result_type operator ()(smoc_sysc_port &p) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpPort::operator ()(smoc_sysc_port &) [BEGIN]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    SGX::Port port(getName(&p), getId(&p));
    port.direction() = p.isInput() ? SGX::Port::In : SGX::Port::Out;
    sassert(psv.ports.insert(std::make_pair(&p, &port)).second);
    psv.proc.ports().push_back(port);
    if (p.getActorPort() == &p) {
#ifdef SYSTEMOC_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << getName(&p) << " => expectedChannelConnections";
      }
#endif //defined(SYSTEMOC_DEBUG)
      for (smoc_sysc_port::Interfaces::const_iterator iter = p.get_interfaces().begin();
           iter != p.get_interfaces().end();
           ++iter) {
#ifdef SYSTEMOC_DEBUG
        if (outDbg.isVisible(Debug::Low)) {
          outDbg << " " << reinterpret_cast<void *>(*iter);
        }
#endif //defined(SYSTEMOC_DEBUG)
        sassert(psv.epc.expectedChannelConnections.insert(
          std::make_pair(*iter, &port)).second);
      }
#ifdef SYSTEMOC_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << std::endl;
      }
#endif //defined(SYSTEMOC_DEBUG)
    }
    if (p.getParentPort()) {
#ifdef SYSTEMOC_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << getName(&p) << " => expectedOuterPorts " << p.getParentPort()->name() << std::endl;
      }
#endif //defined(SYSTEMOC_DEBUG)
      sassert(psv.epc.expectedOuterPorts.insert(
        std::make_pair(p.getParentPort(), &port)).second);
    }
    SCPortBase2Port::iterator iter = psv.expectedOuterPorts.find(&p);
    if (iter != psv.expectedOuterPorts.end()) {
#ifdef SYSTEMOC_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << " => handeled expectedOuterPorts " << iter->second->name() << " connected to outer port " << getName(&p) << std::endl;
      }
#endif //defined(SYSTEMOC_DEBUG)
      iter->second->otherPorts().insert(port.toPtr());
      psv.expectedOuterPorts.erase(iter); // handled it!
    }
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpPort::operator ()(smoc_sysc_port &) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }

  result_type operator ()(sc_core::sc_port_base &p) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpPort::operator ()(sc_port_base &) [BEGIN]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    ChanAdapterBase *chanAdapterBase = dynamic_cast<ChanAdapterBase *>(p.get_interface());
    if (chanAdapterBase != nullptr) {
      SGX::Port port(p.name());
      sassert(psv.ports.insert(std::make_pair(&p, &port)).second);
      psv.proc.ports().push_back(port);
#ifdef SYSTEMOC_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << p.name() << " => unclassifiedPorts" << std::endl;
      }
#endif //defined(SYSTEMOC_DEBUG)
      sassert(psv.epc.unclassifiedPorts.insert(
        std::make_pair(&chanAdapterBase->getIface(), &port)).second);
      SCInterface2Port::iterator iter =
        psv.unclassifiedPorts.find(&chanAdapterBase->getIface());
      if (iter != psv.unclassifiedPorts.end()) {
        port.innerConnectedPort() = iter->second;
        psv.unclassifiedPorts.erase(iter); // handled it!
      }
    } else {
#ifdef SYSTEMOC_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << p.name() << " => ignore" << std::endl;
      }
#endif //defined(SYSTEMOC_DEBUG)
    }
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpPort::operator ()(sc_port_base &) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }

};

class DumpFifoBase: public NamedIdedObjAccess {
protected:
  GraphSubVisitor &gsv;
public:
  DumpFifoBase(GraphSubVisitor &gsv)
    : gsv(gsv) {}

  void connectPort(SGX::Port &pChan, sc_core::sc_interface *sci, SGX::Port::Direction d) {
    {
      SCInterface2Port::iterator iter =
        gsv.expectedChannelConnections.find(sci);
      if (iter != gsv.expectedChannelConnections.end()) {
#ifdef SYSTEMOC_DEBUG
        if (outDbg.isVisible(Debug::Low)) {
          outDbg << "DumpFifoBase::connectPort handeled expectedChannelConnection " << reinterpret_cast<void *>(iter->first) << std::endl;
        }
#endif //defined(SYSTEMOC_DEBUG)
        pChan.actorPort() = iter->second;
        gsv.expectedChannelConnections.erase(iter); // handled it!
        return;
      }
    }
    // Now handle smoc <-> SystemC port adapter stuff
    {
      SCInterface2Port::iterator iter =
        gsv.unclassifiedPorts.find(sci);
      if (iter != gsv.unclassifiedPorts.end()) {
#ifdef SYSTEMOC_DEBUG
        if (outDbg.isVisible(Debug::Low)) {
          outDbg << "DumpFifoBase::connectPort handeled unclassifiedPort " << reinterpret_cast<void *>(iter->first) << std::endl;
        }
#endif //defined(SYSTEMOC_DEBUG)
        iter->second->direction() = d;
        pChan.actorPort() = iter->second;
        gsv.unclassifiedPorts.erase(iter); // handled it!
        return;
      }
    }
    std::cerr << "WTF?! " << pChan.name() << " sc_interface not found!" << std::endl;
  }

  void registerPorts(SGX::Channel &channel, Chan &rc) {
    for (Chan::EntryMap::const_iterator iter = rc.getEntries().begin();
         iter != rc.getEntries().end();
         ++iter) {
      SGX::Port p(Concat(getName(&rc))(".in"));
      p.direction() = SGX::Port::In;
      channel.ports().push_back(p);
      connectPort(p, iter->first, SGX::Port::Out);
    }
    for (Chan::OutletMap::const_iterator iter = rc.getOutlets().begin();
         iter != rc.getOutlets().end();
         ++iter) {
      SGX::Port p(Concat(getName(&rc))(".out"));
      p.direction() = SGX::Port::Out;
      channel.ports().push_back(p);
      connectPort(p, iter->first, SGX::Port::In);
    }
  }
};

class DumpFifo
: public DumpFifoBase {
public:
  typedef void result_type;
protected:
  class ImplDumpingInitialTokens
  : public IfDumpingInitialTokens {
     SGX::Fifo &fifo;
  public:
    ImplDumpingInitialTokens(SGX::Fifo &fifo)
      : fifo(fifo) {}
    
    void setType(const std::string &str) {
      fifo.type() = str;
    }
    
    void addToken(const std::string &str) {
      SGX::Token t; t.value() = str;
      fifo.initialTokens().push_back(t);
    }
  };
public:
  DumpFifo(GraphSubVisitor &gsv)
    : DumpFifoBase(gsv) {}

  result_type operator ()(smoc_fifo_chan_base &p) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpFifo::operator ()(...) [BEGIN] for " << getName(&p) << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    SGX::Fifo fifo(getName(&p), getId(&p));
    // set some attributes
    fifo.size() = p.depthCount();
    gsv.pg.processes().push_back(fifo);
    registerPorts(fifo, p);
    
    ImplDumpingInitialTokens itf(fifo);
    p.dumpInitialTokens(&itf);
    
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpFifo::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }
};

class DumpMultiportFifo
: public DumpFifoBase {
public:
  typedef void result_type;
protected:
  class ImplDumpingInitialTokens
  : public IfDumpingInitialTokens {
     SGX::MultiportFifo &fifo;
  public:
    ImplDumpingInitialTokens(SGX::MultiportFifo &fifo)
      : fifo(fifo) {}
    
    void setType(const std::string &str) {
      fifo.type() = str;
    }
    
    void addToken(const std::string &str) {
      SGX::Token t; t.value() = str;
      fifo.initialTokens().push_back(t);
    }
  };
public:
  DumpMultiportFifo(GraphSubVisitor &gsv)
    : DumpFifoBase(gsv) {}

  result_type operator ()(smoc_multireader_fifo_chan_base &p) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpMultiportFifo::operator ()(...) [BEGIN] for " << getName(&p) << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    SGX::MultiportFifo fifo(getName(&p), getId(&p));
    // set some attributes
    fifo.size() = p.depthCount();
    gsv.pg.processes().push_back(fifo);
    registerPorts(fifo, p);
    
    ImplDumpingInitialTokens itf(fifo);
    p.dumpInitialTokens(&itf);
    
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpMultiportFifo::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }
};

class DumpMultiplexFifo
: public DumpFifoBase {
  typedef DumpMultiplexFifo this_type;
  typedef DumpFifoBase      base_type;
public:
  typedef void result_type;
protected:
  class ImplDumpingInitialTokens
  : public IfDumpingInitialTokens {
     SGX::MultiplexFifo &fifo;
  public:
    ImplDumpingInitialTokens(SGX::MultiplexFifo &fifo)
      : fifo(fifo) {}
    
    void setType(const std::string &str) {
      fifo.type() = str;
    }
    
    void addToken(const std::string &str) {
      SGX::ColoredToken t(0xDEADBEEF); t.value() = str;
      fifo.initialTokens().push_back(t);
    }
  };
public:
  DumpMultiplexFifo(GraphSubVisitor &gsv)
    : DumpFifoBase(gsv) {}

  void registerPorts(SGX::MultiplexFifo &channel, smoc_multiplex_fifo_chan_base &rc) {
    // for the non-colored ones
    base_type::registerPorts(channel, rc);
    // Now the colored ports
    for (smoc_multiplex_fifo_chan_base::VEntryMap::const_iterator iter = rc.getVEntries().begin();
         iter != rc.getVEntries().end();
         ++iter) {
      SGX::ColoredPort p(Concat(getName(&rc))(".in"), iter->first);
      p.direction() = SGX::Port::In;
      channel.ports().push_back(p);
      connectPort(p, iter->second, SGX::Port::Out);
    }
    for (smoc_multiplex_fifo_chan_base::VOutletMap::const_iterator iter = rc.getVOutlets().begin();
         iter != rc.getVOutlets().end();
         ++iter) {
      SGX::ColoredPort p(Concat(getName(&rc))(".out"), iter->first);
      p.direction() = SGX::Port::Out;
      channel.ports().push_back(p);
      connectPort(p, iter->second, SGX::Port::In);
    }
  }

  result_type operator ()(smoc_multiplex_fifo_chan_base &p) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpMultiplexFifo::operator ()(...) [BEGIN] for " << getName(&p) << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    SGX::MultiplexFifo fifo(getName(&p), getId(&p));
    // set some attributes
    fifo.size() = p.depthCount();
    gsv.pg.processes().push_back(fifo);
    registerPorts(fifo, p);
    
    ImplDumpingInitialTokens itf(fifo);
    p.dumpInitialTokens(&itf);
    
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpMultiplexFifo::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }
};

class DumpResetNet
: public DumpFifoBase {
public:
  typedef void result_type;
protected:
public:
  DumpResetNet(GraphSubVisitor &gsv)
    : DumpFifoBase(gsv) {}

  result_type operator ()(smoc_reset_chan &p) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpResetNet::operator ()(...) [BEGIN] for " << getName(&p) << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    SGX::ResetNet fifo(getName(&p), getId(&p));
//  // set some attributes
//  fifo.size() = p.depthCount();
    gsv.pg.processes().push_back(fifo);
    registerPorts(fifo, p);
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpResetNet::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }
};

class DumpActor: public NamedIdedObjAccess {
public:
  typedef void result_type;
protected:
  GraphSubVisitor &gsv;

  struct TransitionInfo {
    SGX::Action::Ptr  actionPtr;
    SGX::ASTNode::Ptr astPtr;
  };
public:
  DumpActor(GraphSubVisitor &gsv)
    : gsv(gsv) {}

  result_type operator ()(smoc_actor &a) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpActor::operator ()(...) [BEGIN] for " << getName(&a) << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    SGX::Actor actor(getName(&a), getId(&a));
    actor.cxxClass() = typeid(a).name();
    ActorSubVisitor sv(gsv.ctx, gsv, actor);
    recurse(sv, a);
    // Dump constructor parameters
    {
      SGX::ParameterList::Ref pl = actor.constructorParameters();
      
      for (ParamInfoList::const_iterator pIter = a.constrArgs.pil.begin();
           pIter != a.constrArgs.pil.end();
           ++pIter) {
        SGX::Parameter parm(pIter->type, pIter->value);
        parm.name() = pIter->name;
        pl.push_back(parm);
      }
    }
    // Dump firingFSM
    {
      typedef std::map<const RuntimeState *, SGX::FiringState::Ptr> StateMap;
      typedef std::map<void *, TransitionInfo>                      TransitionInfoMap;
      
      SGX::FiringFSM              sgxFSM;
      FiringFSMImpl              *smocFSM       = a.getFiringFSM();
      SGX::FiringStateList::Ref   sgxStateList  = sgxFSM.states();
      StateMap                    stateMap;
      TransitionInfoMap           transitionInfoMap;
      
      ActionNGXVisitor            actionVisitor;
      ExprNGXVisitor              exprVisitor(sv.ports);
      
      const RuntimeStateSet      &smocStates    = smocFSM->getStates();
      // Create states
      for (RuntimeStateSet::const_iterator sIter = smocStates.begin();
           sIter != smocStates.end();
           ++sIter) {
        std::string stateName = getName(*sIter);
        // Eleminate actor name from state name, e.g.,
        // sqrroot.a1:smoc_firing_state_0 => smoc_firing_state_0.
        std::string::size_type colonPos = stateName.find(':');
        if (colonPos != std::string::npos)
          stateName = stateName.substr(colonPos+1);
        SGX::FiringState sgxState(stateName, getId(*sIter));
        sassert(stateMap.insert(std::make_pair(*sIter, &sgxState)).second);
        sgxStateList.push_back(sgxState);
      }
      // Setup initial state
      {
        StateMap::const_iterator iIter = stateMap.find(smocFSM->getInitialState());
        assert(iIter != stateMap.end());
        sgxFSM.startState() = iIter->second;
      }
      // Insert transitions
      for (StateMap::iterator sIter = stateMap.begin();
           sIter != stateMap.end();
           ++sIter) {
        const RuntimeState             &smocState = *sIter->first;
        SGX::FiringState::Ref           sgxState  = *sIter->second;
        const RuntimeTransitionList    &smocTrans = smocState.getTransitions();
        SGX::FiringTransitionList::Ref  sgxTrans  = sgxState.outTransitions();
        
        for (RuntimeTransitionList::const_iterator tIter = smocTrans.begin();
             tIter != smocTrans.end();
             ++tIter) {
          SGX::FiringTransition sgxTran(getId(&*tIter));
          sgxTrans.push_back(sgxTran);
          StateMap::const_iterator dIter = stateMap.find(tIter->getDestState());
          assert(dIter != stateMap.end());
          sgxTran.dstState() = dIter->second;
          std::pair<TransitionInfoMap::iterator, bool> inserted =
            transitionInfoMap.insert(std::make_pair(tIter->getID(), TransitionInfo()));
          if (inserted.second) {
            inserted.first->second.actionPtr = boost::apply_visitor(actionVisitor,
              const_cast<smoc_action &>(tIter->getAction()));
            if (gsv.ctx.simCTX->isSMXDumpingASTEnabled()) {
              boost::scoped_ptr<SGX::ASTNode> astNode(
                Expr::evalTo(exprVisitor, tIter->getExpr()));
              inserted.first->second.astPtr = astNode->toPtr();
            }
          }
          sgxTran.action()            = inserted.first->second.actionPtr;
          sgxTran.activationPattern() = inserted.first->second.astPtr;
        }
      }
      actor.firingFSM() = &sgxFSM;
    }
    gsv.pg.processes().push_back(actor);
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpActor::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }
};

class DumpSCModule {
public:
  typedef void result_type;
protected:
  GraphSubVisitor &gsv;
public:
  DumpSCModule(GraphSubVisitor &gsv)
    : gsv(gsv) {}

  result_type operator ()(sc_core::sc_module &a) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpSCModule::operator ()(...) [BEGIN] for " << a.name() << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    SGX::SCModule scModule(a.name());
    scModule.cxxClass() = typeid(a).name();
    gsv.pg.processes().push_back(scModule);
    ProcessSubVisitor sv(gsv.ctx, gsv, scModule);
    recurse(sv, a);
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpSCModule::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }
};

class DumpGraph: public NamedIdedObjAccess {
public:
  typedef void result_type;
protected:
  GraphSubVisitor &gsv;
public:
  DumpGraph(GraphSubVisitor &gsv)
    : gsv(gsv) {}

  result_type operator ()(GraphBase &g) {
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpGraph::operator ()(...) [BEGIN] for " << getName(&g) << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    SGX::RefinedProcess rp(Concat(getName(&g))("_rp"));
    gsv.pg.processes().push_back(rp);
    SGX::ProblemGraph   pg(getName(&g), getId(&g));
    pg.cxxClass() = typeid(g).name();
    rp.refinements().push_back(pg);
    GraphSubVisitor sv(gsv.ctx, gsv, rp, pg);
    recurse(sv, g);
#ifdef SYSTEMOC_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpGraph::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
  }
};

void ProcessSubVisitor::operator ()(smoc_sysc_port &obj) {
  DumpPort(*this)(obj);
}

void ProcessSubVisitor::operator ()(sc_core::sc_port_base &obj) {
  DumpPort(*this)(obj);
}

void GraphSubVisitor::operator ()(GraphBase &obj) {
  DumpGraph(*this)(obj);
}

void GraphSubVisitor::operator ()(smoc_actor &obj) {
  DumpActor(*this)(obj);
}

void GraphSubVisitor::operator ()(sc_core::sc_module &obj) {
  DumpSCModule(*this)(obj);
}

void GraphSubVisitor::operator ()(smoc_fifo_chan_base &obj) {
  DumpFifo(*this)(obj);
}

void GraphSubVisitor::operator ()(smoc_multireader_fifo_chan_base &obj) {
  DumpMultiportFifo(*this)(obj);
}

void GraphSubVisitor::operator ()(smoc_multiplex_fifo_chan_base &obj) {
  DumpMultiplexFifo(*this)(obj);
}

void GraphSubVisitor::operator ()(smoc_reset_chan &obj) {
  DumpResetNet(*this)(obj);
}

GraphSubVisitor::~GraphSubVisitor() {
  // Kick expectedChannelConnections one layer up
  epc.expectedChannelConnections.insert(
    expectedChannelConnections.begin(),
    expectedChannelConnections.end());
  expectedChannelConnections.clear();
}

void dumpSMX(std::ostream &file, SimulationContextSMXDumping *simCTX, GraphBase &g) {
  SGX::NetworkGraphAccess ngx;
  SMXDumpCTX              ctx(simCTX);
  ExpectedPortConnections epc;
  SGX::RefinedProcess     rp;
  SGX::ProblemGraph       pg(g.name(), IdedObjAccess::getId(&g));
  
  rp.refinements().push_back(pg);
  GraphSubVisitor sv(ctx,epc,rp,pg);
  recurse(sv, g);
  ngx.problemGraphPtr() = &pg;
  ngx.architectureGraphPtr() = SGX::ArchitectureGraph("dummy architecture graph").toPtr(); 
  // There may be dangling ports => erase them or we get an assertion!
  epc.expectedOuterPorts.clear();
//epc.expectedChannelConnections.clear();
  ngx.save(file);
}

} } // namespace smoc::Detail

#endif // SYSTEMOC_ENABLE_SGX
