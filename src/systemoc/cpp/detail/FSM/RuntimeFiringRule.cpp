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

#include "RuntimeFiringRule.hpp"

#include <smoc/detail/DebugOStream.hpp>

#include <tuple>
#include <cassert>
#include <set>
#include <map>

namespace smoc { namespace Detail { namespace FSM {

  typedef SimulatorAPI::FunctionNames FunctionNames;

  class RuntimeFiringRule::GuardVisitor: public ExprVisitor<void> {
  private:
    typedef ExprVisitor<void> base_type;
    typedef GuardVisitor      this_type;
  public:
    GuardVisitor(RuntimeFiringRule &fr)
      : fr(fr) {}

    result_type visitVar(std::string const &name, std::string const &type) {
      return nullptr;
    }
    result_type visitLiteral(std::string const &type, std::string const &value) {
      fr.guardComplexity++;
      return nullptr;
    }
    result_type visitMemGuard(
        std::string const &name, std::string const &cxxType,
        std::string const &reType, ParamInfoList const &params) {
      fr.guardNames.push_back(name);
      fr.guardComplexity++;
      return nullptr;
    }
    result_type visitEvent(smoc_event_waiter &e, std::string const &name) {
      *fr.ioPatternWaiter &= e;
      return nullptr;
    }
    result_type visitToken(PortBase &p, size_t n){
      return nullptr;
    }
    result_type visitComm(PortBase &p, size_t c, size_t r) {
      if (p.isInput())
        fr.portInInfos.push_back(PortInInfo(static_cast<PortInBase &>(p), c, r));
      else {
        fr.portOutInfos.push_back(PortOutInfo(static_cast<PortOutBase &>(p), c));
        assert(c == r);
      }
      return nullptr;
    }
    result_type visitUnOp(OpUnT op,
        boost::function<result_type (base_type &)> e){
      e(*this);
      return nullptr;
    }
    result_type visitBinOp(OpBinT op,
        boost::function<result_type (base_type &)> a,
        boost::function<result_type (base_type &)> b){
      a(*this);
      b(*this);
      return nullptr;
    }
  private:
    RuntimeFiringRule &fr;
  };

  static
  smoc_event_and_list *getCAP(const smoc_event_and_list &ap) {
    typedef std::set<smoc_event_and_list> Cache;
    static Cache* cache = new Cache();
    return &const_cast<smoc_event_and_list &>(*cache->insert(ap).first);
  }

  RuntimeFiringRule::RuntimeFiringRule(smoc_guard const &g, smoc_action const &f)
    : smoc_firing_rule(g,f)
    , guardComplexity(0)
    , ioPatternWaiter(nullptr)
  {
    smoc_event_and_list tmp;
    ioPatternWaiter = &tmp;
    GuardVisitor visitor(*this);
    Expr::evalTo(visitor, getGuard());
    for (PortInInfo portInfo : getPortInInfos()) {
      tmp &= static_cast<PortInBase &>(portInfo.port).blockEvent(portInfo.required);
    }
    for (PortOutInfo portInfo : getPortOutInfos()) {
      tmp &= static_cast<PortOutBase &>(portInfo.port).blockEvent(portInfo.produced);
    }
    ioPatternWaiter = getCAP(tmp);
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  FunctionNames RuntimeFiringRule::getGuardNames() const {
    return guardNames;
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  size_t        RuntimeFiringRule::getGuardComplexity() const {
    return guardComplexity;
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  FunctionNames RuntimeFiringRule::getActionNames() const {
    FunctionNames actionNames;

    for (smoc_func_call const &f : getAction()) {
      actionNames.push_back(f.getFuncName());
    }
    return actionNames;
  }

  RuntimeFiringRule::~RuntimeFiringRule() {
  }

} } } // namespace smoc::Detail::FSM
