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
#include <smoc/detail/TraceLog.hpp>
#include <smoc/detail/DebugOStream.hpp>

#ifdef MAESTRO_ENABLE_POLYPHONIC
# include <Maestro/PolyphoniC/polyphonic_smoc_func_call.h>
#endif //MAESTRO_ENABLE_POLYPHONIC

smoc_action merge(const smoc_action &a, const smoc_action &b) {
  if(a.empty())
    return b;
  if(b.empty())
    return a;

  smoc_action ret = a;
  for(smoc_action::const_iterator i = b.begin();
      i != b.end();
      ++i)
  {
    ret.push_back(*i);
  }
  return ret;
}

ActionVisitor::ActionVisitor(result_type dest)
  : dest(dest) {}

ActionVisitor::result_type ActionVisitor::operator()(const smoc_action &f) const {
  // Function call
  for(smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&*i);
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
           << i->getFuncName() << "\">" << std::endl;
    }
#endif // SYSTEMOC_ENABLE_DEBUG
  
    (*i)();

#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "</action>" << std::endl;
    }
#endif // SYSTEMOC_ENABLE_DEBUG
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
  }
  return dest;
}

#ifdef MAESTRO_ENABLE_POLYPHONIC

smoc::dMM::TransitionOnThreadVisitor::TransitionOnThreadVisitor(result_type dest, MetaMap::Transition* tr)
  : dest(dest), transition(tr)
{}

smoc::dMM::TransitionOnThreadVisitor::result_type smoc::dMM::TransitionOnThreadVisitor::operator()(const smoc_action& f) const
{
  boost::thread privateThread;

  bool hasWaitTime = false;

  for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
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
    for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
# ifdef SYSTEMOC_ENABLE_DEBUG
      if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
        smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
          << i->getFuncName() << "\">" << std::endl;
      }
# endif // SYSTEMOC_ENABLE_DEBUG
      (*i)();
# ifdef SYSTEMOC_ENABLE_DEBUG
      if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
        smoc::Detail::outDbg << "</action>" << std::endl;
      }
# endif // SYSTEMOC_ENABLE_DEBUG
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
      getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
    }
  }
  
  return dest;
}

void smoc::dMM::TransitionOnThreadVisitor::executeTransition(const smoc_action& f) const
{
  // Function call
  for (smoc_action::const_iterator i = f.begin(); i != f.end(); ++i) {
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
# ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
        << i->getFuncName() << "\">" << std::endl;
    }
# endif // SYSTEMOC_ENABLE_DEBUG

    (*i)();

# ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "</action>" << std::endl;
    }
# endif // SYSTEMOC_ENABLE_DEBUG
# ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
# endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
  }

  transition->notifyThreadDone();
}

#endif //MAESTRO_ENABLE_POLYPHONIC
