// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
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

#include <systemoc/detail/smoc_func_call.hpp>
#include <systemoc/detail/smoc_firing_rules_impl.hpp>
#include <smoc/detail/TraceLog.hpp>
#include <smoc/detail/DebugOStream.hpp>

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <Maestro/PolyphoniC/polyphonic_smoc_func_call.h>
#endif //MAESTRO_ENABLE_POLYPHONIC

smoc_action merge(const smoc_action& a, const smoc_action& b) {
  if(const smoc_func_call_list* _a = boost::get<smoc_func_call_list>(&a)) {
    if(_a->empty()) return b;
    
    if(const smoc_func_call_list* _b = boost::get<smoc_func_call_list>(&b)) {
      smoc_func_call_list ret = *_a;
      for(smoc_func_call_list::const_iterator i = _b->begin();
          i != _b->end(); ++i)
      {
        ret.push_back(*i);
      }
      return ret;
    }
  }
  if(const smoc_func_call_list* _b = boost::get<smoc_func_call_list>(&b)) {
    if(_b->empty()) return a;
  }  
  assert(0);
}

ActionVisitor::ActionVisitor(RuntimeState *dest)
  : dest(dest) {}

RuntimeState *ActionVisitor::operator()(const smoc_func_call_list& f) const {
  // Function call
  for(smoc_func_call_list::const_iterator i = f.begin(); i != f.end(); ++i) {
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&*i);
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
           << i->getFuncName() << "\">" << std::endl;
    }
#endif // SYSTEMOC_DEBUG
  
    (*i)();

#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "</action>" << std::endl;
    }
#endif // SYSTEMOC_DEBUG
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
  }
  return dest;
}

#ifdef MAESTRO_ENABLE_POLYPHONIC

smoc::dMM::TransitionOnThreadVisitor::TransitionOnThreadVisitor(RuntimeState* dest, MetaMap::Transition* tr)
  : dest(dest), transition(tr)
{}

RuntimeState* smoc::dMM::TransitionOnThreadVisitor::operator()(const smoc_func_call_list& f) const
{
  boost::thread privateThread;

  bool hasWaitTime = false;

  for (smoc_func_call_list::const_iterator i = f.begin(); i != f.end(); ++i) {
    string name = i->getFuncName();
    
    if (i->isWaitCall()) {
      hasWaitTime = true;
    }
  }

  if (!hasWaitTime) {
    privateThread = boost::thread(&TransitionOnThreadVisitor::executeTransition, this, f);
    transition->waitThreadDone();
    //privateThread.join();
  } else {
    // Function call
    for (smoc_func_call_list::const_iterator i = f.begin(); i != f.end(); ++i) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
# ifdef SYSTEMOC_DEBUG
      if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
        smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
          << i->getFuncName() << "\">" << std::endl;
      }
# endif // SYSTEMOC_DEBUG
      (*i)();
# ifdef SYSTEMOC_DEBUG
      if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
        smoc::Detail::outDbg << "</action>" << std::endl;
      }
# endif // SYSTEMOC_DEBUG
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
    }
  }
  
  return dest;
}

void smoc::dMM::TransitionOnThreadVisitor::executeTransition(const smoc_func_call_list& f) const
{
  // Function call
  for (smoc_func_call_list::const_iterator i = f.begin(); i != f.end(); ++i) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
# ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
        << i->getFuncName() << "\">" << std::endl;
    }
# endif // SYSTEMOC_DEBUG

    (*i)();

# ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "</action>" << std::endl;
    }
# endif // SYSTEMOC_DEBUG
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
  }

  transition->notifyThreadDone();
}

#endif //MAESTRO_ENABLE_POLYPHONIC

#if defined(SYSTEMOC_ENABLE_VPC) || defined(SYSTEMOC_ENABLE_TRANSITION_TRACE)
namespace smoc { namespace Detail {

ActionNameVisitor::ActionNameVisitor(FunctionNames & names)
  : functionNames(names) {}

void ActionNameVisitor::operator()(const smoc_func_call_list &f) const {
  for (smoc_func_call_list::const_iterator i = f.begin(); i != f.end(); ++i) {
    functionNames.push_back(i->getFuncName());
  }
}

} } // namespace smoc::Detail
#endif // defined(SYSTEMOC_ENABLE_VPC) || defined(SYSTEMOC_ENABLE_TRANSITION_TRACE)
