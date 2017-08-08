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

#include <map>
#include <utility>
#include <memory>

#include <CoSupport/compatibility-glue/nullptr.h>
#include <CoSupport/String/Concat.hpp>

#include <smoc/detail/DebugOStream.hpp>

#include <boost/variant/static_visitor.hpp>

//#define SYSTEMOC_DEBUG

namespace smoc { namespace Detail {

namespace SGX = SystemCoDesigner::SGX;

using CoSupport::String::Concat;

SimulationContextSMXImporting::SimulationContextSMXImporting()
  : importSMXFile(nullptr)
  {}

class ProcessVisitor
  : public boost::static_visitor<void> {
public:
  // Only match "RefinedProcess"es.
  result_type operator()(SGX::RefinedProcess const &p);

  // This is the fallback operator that matches all else.
  result_type operator()(SGX::Process const &p);

};

void iterateGraphs(SGX::ProblemGraph const &pg, ProcessVisitor &pv) {
  SGX::ProcessList::ConstRef processList = pg.processes();

  for (SGX::Process::ConstRef proc : processList)
    apply_visitor(pv, proc);
}

ProcessVisitor::result_type ProcessVisitor::operator()(SGX::RefinedProcess const &p) {
  assert(p.refinements().size() == 1);
  SGX::ProblemGraph const &pg = p.refinements().front();
  if (pg.firingFSM()) {
    std::cerr << pg.name() << " is a cluster!" << std::endl;
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
  
  ProcessVisitor pv;

  iterateGraphs(ngx.problemGraph(), pv);

  simCTX->pNGX = ngx.toPtr();
}

} } // namespace smoc::Detail

#endif // SYSTEMOC_ENABLE_SGX
