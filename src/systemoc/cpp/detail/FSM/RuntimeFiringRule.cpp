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

#include <cassert>

namespace smoc { namespace Detail { namespace FSM {

  typedef SimulatorAPI::FunctionNames FunctionNames;

  namespace {

    class GuardNameVisitor: public ExprVisitor<void> {
    public:
      typedef ExprVisitor<void>            base_type;
      typedef GuardNameVisitor             this_type;

    public:
      GuardNameVisitor(FunctionNames &names)
        : functionNames(names)
        , complexity(0) {}

      int getComplexity(){
        return complexity;
      }
      result_type visitVar(const std::string &name, const std::string &type){
        return nullptr;
      }
      result_type visitLiteral(const std::string &type,
          const std::string &value){
        if (type == "m") {
          val.push_back(value);
        }
        complexity++;
        return nullptr;
      }
      result_type visitMemGuard(
          const std::string &name, const std::string& cxxType,
          const std::string &reType, const ParamInfoList &params){
        functionNames.push_back(name);
        complexity++;
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
      result_type visitComm(PortBase &p, size_t c, size_t r) {
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
      FunctionNames             &functionNames;
      int                        complexity;
      std::vector<std::string>   val;
    };

  } // namespace anonymous

  RuntimeFiringRule::RuntimeFiringRule(smoc_guard const &g, smoc_action const &f)
    : smoc_firing_rule(g,f), ioPatternWaiter(nullptr)
  {




  }



  /// Implement SimulatorAPI::FiringRuleInterface
  void RuntimeFiringRule::freeInputs() {


  }

  /// Implement SimulatorAPI::FiringRuleInterface
  void RuntimeFiringRule::releaseOutputs() {


  }

  /// Implement SimulatorAPI::FiringRuleInterface
  FunctionNames RuntimeFiringRule::getGuardNames() const {
    FunctionNames guardNames;

    GuardNameVisitor visitor(guardNames);
    Expr::evalTo(visitor, getGuard());
    return guardNames;
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  size_t        RuntimeFiringRule::getGuardComplexity() const {
    FunctionNames guardNames;

    GuardNameVisitor visitor(guardNames);
    Expr::evalTo(visitor, getGuard());
    return visitor.getComplexity();
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  FunctionNames RuntimeFiringRule::getActionNames() const {
    FunctionNames actionNames;

    for (smoc_func_call const &f : getAction()) {
      actionNames.push_back(f.getFuncName());
    }
    return actionNames;
  }

  void RuntimeFiringRule::end_of_elaboration() {
    assert(!ioPatternWaiter);

    IOPattern ioPattern;
    smoc::Expr::evalTo<smoc::Expr::Sensitivity>(getGuard(), ioPattern);
    ioPattern.finalise();
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Low)) {
      smoc::Detail::outDbg << "=> " << ioPattern << std::endl;
    }
#endif //defined(SYSTEMOC_DEBUG)
    ioPatternWaiter = ioPattern.getWaiter();
  }

} } } // namespace smoc::Detail::FSM
