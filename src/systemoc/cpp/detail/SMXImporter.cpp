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

#include <CoSupport/SmartPtr/RefCountObject.hpp>

#include "SMXImporter.hpp"
#include "SimulationContext.hpp"

#ifdef SYSTEMOC_ENABLE_SGX

#include <CoSupport/compatibility-glue/nullptr.h>
#include <CoSupport/String/Concat.hpp>

#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/PortBase.hpp>
#include <smoc/smoc_graph.hpp>

#include <sgxutils/ASTTools.hpp>

#include <boost/variant/static_visitor.hpp>

#include <map>
#include <utility>
#include <memory>
#include <string>

//#define SYSTEMOC_DEBUG

namespace smoc { namespace Detail {

namespace SGX = SystemCoDesigner::SGXUtils;

using CoSupport::String::Concat;

SimulationContextSMXImporting::SimulationContextSMXImporting()
  : importSMXFile(nullptr)
  {}

class ASTEvaluator
: public SGX::ASTTools::ASTEvaluator<
    ASTEvaluator,
    boost::mpl::vector<Expr::Ex<bool>::type, PortBase *, int> > {

private:
  IdPool &idPool;

public:
  ASTEvaluator(IdPool &idPool)
    : idPool(idPool) {}

  inline
  result_type translateASTNode(const SGX::ASTLeafNode &)
    { return boost::blank(); }

  inline
  result_type translateASTNode(const SGX::ASTNodePortTokens &portTokens) {
    PortBase *smocClusterPort =
        dynamic_cast<PortBase *>(idPool.getNodeById(portTokens.port()->id()));
    assert(smocClusterPort);
    return smocClusterPort;
  }

  inline
  result_type translateASTNode(const SGX::ASTNodeLiteral &literal) {
    return std::stoi(literal.value());
  }

  template <typename T2>
  inline
  result_type translateASTNode(const SGX::ASTUnNode &, const T2 &)
    { return boost::blank(); }

  template <typename T2, typename T3>
  inline
  result_type translateASTNode(const SGX::ASTBinNode &, const T2 &, const T3 &)
    { return boost::blank(); }

  inline
  result_type translateASTNode(const SGX::ASTNodeBinOpLAnd &, boost::blank, Expr::Ex<bool>::type const &rhs)
    { return rhs; }

  inline
  result_type translateASTNode(const SGX::ASTNodeBinOpLAnd &, Expr::Ex<bool>::type const &lhs, boost::blank)
    { return lhs; }

  inline
  result_type translateASTNode(const SGX::ASTNodeBinOpLAnd &, Expr::Ex<bool>::type const &lhs, Expr::Ex<bool>::type const &rhs)
    { return lhs && rhs; }

  inline
  result_type translateASTNode(const SGX::ASTNodeBinOpGe &, PortBase *p, int request) {
    // FIXME: This should reuse smoc_port_in<T>::communicate / smoc_port_out<T>::communicate.
    return
      Expr::comm(*p,
                 Expr::DLiteral<size_t>(0),
                 Expr::DLiteral<size_t>(request))
      && //FIXME: Expr::comm knows n and m -> should remove Expr::portTokens
      Expr::portTokens(*p) >= request;
  }

};

class QSSCluster
: public smoc_graph
, public NamedIdedObjAccess
{
private:
  smoc_firing_state initState;
public:
  QSSCluster(sc_core::sc_module_name name, SGX::RefinedProcess const &rp)
    : smoc_graph(name, initState)
    , initState("initState")
  {
    std::cerr << "Creating cluster " << this->name() << std::endl;
    for (SGX::Port::ConstRef sgxPort : rp.ports()) {
      PortBase *smocActorPort =
          dynamic_cast<PortBase *>(getSimCTX()->getIdPool().getNodeById(sgxPort.actorPort()->id()));
      assert(smocActorPort != nullptr);
      PortBase *smocClusterPort = smocActorPort->copyPort(sgxPort.name().get().c_str(), sgxPort.id());
      std::cerr << "Creating port " << smocClusterPort->name() << " of cluster" << std::endl;
    }
    SGX::FiringFSM::ConstRef sgxFSM = *rp.refinements().front().firingFSM();

    typedef std::map<NgId, smoc_firing_state::Ptr> SMoCStates;
    SMoCStates smocStates;

    for (SGX::FiringState::ConstRef sgxState : sgxFSM.states()) {
      bool success = smocStates.insert(std::make_pair(
              sgxState.id(),
              sgxFSM.startState() == sgxState.toPtr()
               ? initState.toPtr()
               : smoc_firing_state(sgxState.name().get()).toPtr())).second;
      assert(success);
    }

    for (SGX::FiringState::ConstRef sgxSrcState : sgxFSM.states()) {
      SMoCStates::iterator srcStateIter = smocStates.find(sgxSrcState.id());
      assert(srcStateIter != smocStates.end());

      for (SGX::FiringTransition::ConstRef sgxTransition : sgxSrcState.outTransitions()) {
        SMoCStates::iterator dstStateIter = smocStates.find(sgxTransition.dstState()->id());
        assert(dstStateIter != smocStates.end());

        ASTEvaluator astEvaluator(getSimCTX()->getIdPool());
        *srcStateIter->second |=
            boost::get<Expr::Ex<bool>::type>(astEvaluator.evaluate(*sgxTransition.activationPattern())) >>
            SMOC_CALL(QSSCluster::flummy) >> *dstStateIter->second;
          ;
      }
    }


  }
protected:
  void flummy() {
    std::cerr << "flummy" << std::endl;
  }
};

class ProcessVisitor
  : public boost::static_visitor<void> {
protected:
  SimulationContext *simCTX;
public:
  ProcessVisitor(SimulationContext *simCTX)
    : simCTX(simCTX) {}

  // Only match "RefinedProcess"es.
  result_type operator()(SGX::RefinedProcess const &rp);

  // This is the fallback operator that matches all else.
  result_type operator()(SGX::Process const &p);

};

void iterateGraphs(SGX::ProblemGraph const &pg, ProcessVisitor &pv) {
  SGX::ProcessList::ConstRef processList = pg.processes();

  for (SGX::Process::ConstRef proc : processList)
    apply_visitor(pv, proc);
}

ProcessVisitor::result_type ProcessVisitor::operator()(SGX::RefinedProcess const &rp) {
  assert(rp.refinements().size() == 1);
  SGX::ProblemGraph const &pg = rp.refinements().front();
  if (pg.firingFSM()) {
    std::cerr << pg.name() << " is a cluster!" << std::endl;
    new QSSCluster(pg.name().get().c_str(), rp);
    // This disables all actors in the cluster. Scheduling
    // of these actors must be performed by the FSM created
    // in QSSCluster.
    for (SGX::Process::ConstRef sgxProc: pg.actors()) {
      NodeBase *smocProcess =
          dynamic_cast<NodeBase *>(simCTX->getIdPool().getNodeById(sgxProc.id()));
      assert(smocProcess);
      smocProcess->setActive(false);
    }
  } else {
    iterateGraphs(pg, *this);
  }
}

ProcessVisitor::result_type ProcessVisitor::operator()(SGX::Process const &p) {
  // Ignore this
}

void importSMX(SimulationContext *simCTX) {
  if (!simCTX->isSMXImportingEnabled())
    return;

  SGX::NetworkGraphAccess ngx(*simCTX->importSMXFile);
  
  ProcessVisitor pv(simCTX);

  iterateGraphs(ngx.problemGraph(), pv);

  simCTX->pNGX = ngx.toPtr();
}

} } // namespace smoc::Detail

#endif // SYSTEMOC_ENABLE_SGX
