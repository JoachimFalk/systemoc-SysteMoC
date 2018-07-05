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

#include <smoc/detail/EventQueue.hpp>
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
      fr.portInfos.push_back(PortInfo(p, c, r));
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

#ifdef SYSTEMOC_ENABLE_VPC
  class RuntimeFiringRule::DIIListener
    : public smoc_event_listener {
  public:
    DIIListener(RuntimeFiringRule *fr)
      : fr(fr) {}
  protected:
    // Tell this listener about an event changing in EventWaiter e
    // (e.g. the waiter was reseted or notified; check e->isActive()(
    virtual void signaled(smoc_event_waiter *e) {
      if (*e)
        fr->freeInputs();
    }

    // The lifetime of the given EventWaiter is over
    virtual void eventDestroyed(smoc_event_waiter *e) {
      assert(!"eventDestroyed must never be called!");
    }
  private:
    RuntimeFiringRule *fr;
  };

  class RuntimeFiringRule::LATListener
    : public smoc_event_listener {
  public:
    LATListener(RuntimeFiringRule *fr)
      : fr(fr) {}
  protected:
    // Tell this listener about an event changing in EventWaiter e
    // (e.g. the waiter was reseted or notified; check e->isActive()(
    virtual void signaled(smoc_event_waiter *e) {
      assert(*e);
      e->delListener(this);
      fr->releaseOutputs();
    }

    // The lifetime of the given EventWaiter is over
    virtual void eventDestroyed(smoc_event_waiter *e) {
      assert(!"eventDestroyed must never be called!");
    }
  private:
    RuntimeFiringRule *fr;
  };
#endif //SYSTEMOC_ENABLE_VPC

  static
  smoc_event_and_list *getCAP(const smoc_event_and_list &ap) {
    typedef std::set<smoc_event_and_list> Cache;
    static Cache* cache = new Cache();
    return &const_cast<smoc_event_and_list &>(*cache->insert(ap).first);
  }

  RuntimeFiringRule::RuntimeFiringRule(smoc_guard const &g, smoc_action const &f)
    : smoc_firing_rule(g,f)
#ifdef SYSTEMOC_ENABLE_VPC
    , diiListener(new DIIListener(this))
    , latListener(new LATListener(this))
#endif //SYSTEMOC_ENABLE_VPC
    , guardComplexity(0)
    , ioPatternWaiter(nullptr)
  {
    smoc_event_and_list tmp;
    ioPatternWaiter = &tmp;
    GuardVisitor visitor(*this);
    Expr::evalTo(visitor, getGuard());
    for (PortInfo portInfo : portInfos) {
      tmp &= portInfo.port.blockEvent(portInfo.required);
    }
    ioPatternWaiter = getCAP(tmp);
#ifdef SYSTEMOC_ENABLE_VPC
    diiEvent->addListener(diiListener);
#endif //SYSTEMOC_ENABLE_VPC
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  void RuntimeFiringRule::freeInputs() {
#ifdef SYSTEMOC_ENABLE_ROUTING
    for (PortInfo portInfo : portInfos)
      if (portInfo.port.isInput()) {
# ifdef SYSTEMOC_ENABLE_DEBUG
        if (outDbg.isVisible(Debug::Low))
          outDbg << __func__ << " " << portInfo.port.name() << ".commFinish(" << portInfo.commited << ")" << std::endl;
# endif //defined(SYSTEMOC_ENABLE_DEBUG)
        portInfo.port.commFinish(portInfo.commited);
      }
#endif //SYSTEMOC_ENABLE_ROUTING
  }

  /// Implement SimulatorAPI::FiringRuleInterface
  void RuntimeFiringRule::releaseOutputs() {
#ifdef SYSTEMOC_ENABLE_ROUTING
    typedef std::map<PortBaseIf *, EventQueue<size_t> > WriteCompleteQueues;

    static
    WriteCompleteQueues writeCompleteQueues;

    assert(!latQueue.empty() && *latQueue.front());
    do {
      for (PortInfo portInfo : portInfos) {
        if (portInfo.port.isOutput()) {
          for (PortBaseIf *pbIf : static_cast<PortOutBase &>(portInfo.port).get_interfaces()) {
            WriteCompleteQueues::iterator iter = writeCompleteQueues.find(pbIf);
            if (iter == writeCompleteQueues.end()) {
              auto success = [pbIf](size_t n) {
# ifdef SYSTEMOC_ENABLE_DEBUG
                if (outDbg.isVisible(Debug::Low))
                  outDbg << __func__ << " " << pbIf->channel << ".commFinish(" << n << ")" << std::endl;
# endif //defined(SYSTEMOC_ENABLE_DEBUG)
                pbIf->commFinish(n);
              };
              auto dropped = [pbIf](size_t n) {
# ifdef SYSTEMOC_ENABLE_DEBUG
                if (outDbg.isVisible(Debug::Low))
                  outDbg << __func__ << " " << pbIf->channel << ".commFinish(" << n << ", true)" << std::endl;
# endif //defined(SYSTEMOC_ENABLE_DEBUG)
                pbIf->commFinish(n, true);
              };
              bool insertStatus;
              std::tie(iter, insertStatus) =
                  writeCompleteQueues.insert(std::make_pair(
                      pbIf,
                      EventQueue<size_t>(success, dropped)));
              assert(insertStatus);
            }
            for (size_t n = portInfo.commited; n > 0; --n) {
              SystemC_VPC::EventPair ep = VpcInterface(this, pbIf).
                  startWrite(1);
              iter->second.addEntry(1, ep.latency);
            }
          }
        }
      }
      latQueue.pop_front();
    } while (!latQueue.empty() && *latQueue.front());
    if (!latQueue.empty())
      latQueue.front()->addListener(latListener);
#endif //SYSTEMOC_ENABLE_ROUTING
  }

  void RuntimeFiringRule::commExec() {
#ifdef SYSTEMOC_ENABLE_ROUTING
    getDiiEvent()->reset();
    for (PortInfo portInfo : portInfos) {
#ifdef SYSTEMOC_ENABLE_DEBUG
      if (outDbg.isVisible(Debug::Low))
        outDbg << __func__ << " " << portInfo.port.name() << ".commStart(" << portInfo.commited << ")" << std::endl;
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
      portInfo.port.commStart(portInfo.commited);
      if (portInfo.port.isInput()) {
        VpcInterface(this, static_cast<PortInBase &>(portInfo.port).get_interface()).
            startVpcRead(portInfo.commited);
      }
    }
    if (latQueue.empty()) {
      getLatEvent()->addListener(latListener);
    }
    latQueue.push_back(getLatEvent());
#else //!SYSTEMOC_ENABLE_ROUTING
    for (PortInfo portInfo : portInfos) {
#ifdef SYSTEMOC_ENABLE_DEBUG
      if (outDbg.isVisible(Debug::Low))
        outDbg << __func__ << " " << portInfo.port.name() << ".commExec(" << portInfo.commited << ")" << std::endl;
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
      portInfo.port.commExec(portInfo.commited);
    }
#endif //!SYSTEMOC_ENABLE_ROUTING
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
#ifdef SYSTEMOC_ENABLE_VPC
    diiEvent->delListener(diiListener);
    delete diiListener;
    if (!latQueue.empty())
      latQueue.front()->delListener(latListener);
    delete latListener;
#endif //SYSTEMOC_ENABLE_VPC
  }

} } } // namespace smoc::Detail::FSM
