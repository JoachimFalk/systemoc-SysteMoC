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

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_func_call.hpp>
#include <systemoc/detail/smoc_firing_rules_impl.hpp>
#include <systemoc/smoc_ast_systemoc.hpp>

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

ActionVisitor::ActionVisitor(RuntimeState* dest, int mode)
  : dest(dest), mode(mode) {}

RuntimeState* ActionVisitor::operator()(smoc_func_call_list& f) const {
  // Function call
  for(smoc_func_call_list::iterator i = f.begin(); i != f.end(); ++i) {
#ifdef SYSTEMOC_TRACE
    TraceLog.traceStartFunction(i->getFuncName());
#endif // SYSTEMOC_TRACE
#ifdef SYSTEMOC_DEBUG
    std::cerr << "    <action type=\"smoc_func_call\" func=\""
              << i->getFuncName() << "\">" << std::endl;
#endif // SYSTEMOC_DEBUG
  
    (*i)();

#ifdef SYSTEMOC_DEBUG
    std::cerr << "    </action>" << std::endl;
#endif // SYSTEMOC_DEBUG
#ifdef SYSTEMOC_TRACE
    TraceLog.traceEndFunction(i->getFuncName());
#endif // SYSTEMOC_TRACE
  }
  return dest;
}

RuntimeState* ActionVisitor::operator()(smoc_func_diverge& f) const {
  // Function call determines next state (Internal use only)
#ifdef SYSTEMOC_DEBUG
  std::cerr << "    <action type=\"smoc_func_diverge\" func=\"???\">"
            << std::endl;
#endif

  RuntimeState* ret = f();

#ifdef SYSTEMOC_DEBUG
  std::cerr << "    </action>" << std::endl;
#endif
  return ret;
}

RuntimeState* ActionVisitor::operator()(smoc_sr_func_pair& f) const {
  // SR GO & TICK calls
#ifdef SYSTEMOC_TRACE
  TraceLog.traceStartFunction(f.go.getFuncName());
#endif
  if(mode & RuntimeTransition::GO) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "    <action type=\"smoc_sr_func_pair\" go=\""
              << f.go.getFuncName() << "\">" << std::endl;
#endif
    f.go();
#ifdef SYSTEMOC_DEBUG
    std::cerr << "    </action>" << std::endl;
#endif
  }
  if(mode & RuntimeTransition::TICK) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "    <action type=\"smoc_sr_func_pair\" tick=\""
              << f.tick.getFuncName() << "\">" << std::endl;
#endif
    f.tick();
#ifdef SYSTEMOC_DEBUG
    std::cerr << "    </action>" << std::endl;
#endif
  }
#ifdef SYSTEMOC_TRACE
  TraceLog.traceEndFunction(f.go.getFuncName());
#endif
  return dest;
}

#ifdef SYSTEMOC_ENABLE_VPC
VPCLinkVisitor::VPCLinkVisitor(const char* name)
  : name(name) {}

SystemC_VPC::FastLink* VPCLinkVisitor::operator()(smoc_func_call_list& f) const {
  std::ostringstream os;

  if(f.begin() == f.end())
    os << "???";

  for(smoc_func_call_list::iterator i = f.begin(); i != f.end(); ++i) {
    if(i != f.begin())
      os << "_";
    os << i->getFuncName();
  }

  return new SystemC_VPC::FastLink(
      SystemC_VPC::Director::getInstance().getFastLink(
        name, os.str()));
}

SystemC_VPC::FastLink* VPCLinkVisitor::operator()(smoc_sr_func_pair& f) const {
  f.tickLink = new SystemC_VPC::FastLink(
      SystemC_VPC::Director::getInstance().getFastLink(
        name, f.tick.getFuncName()));
  return new SystemC_VPC::FastLink(
      SystemC_VPC::Director::getInstance().getFastLink(
        name, f.go.getFuncName()));
}

SystemC_VPC::FastLink* VPCLinkVisitor::operator()(smoc_func_diverge& f) const {
  return new SystemC_VPC::FastLink(
      SystemC_VPC::Director::getInstance().getFastLink(
        name, "smoc_func_diverge"));
}
#endif // SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_SGX
using namespace SystemCoDesigner::SGX;  
using namespace SysteMoC::ActivationPattern;

Action::Ptr ActionNGXVisitor::operator()(smoc_func_call_list& f) const {
  if(f.empty()) return NULL;

  bool single = (++f.begin() == f.end());
  CompoundAction top;

  for(smoc_func_call_list::iterator i = f.begin(); i != f.end(); ++i) {
    Function func;
    func.name() = i->getFuncName();

    for(ParamInfoList::const_iterator pIter = i->getParams().begin();
        pIter != i->getParams().end(); ++pIter)
    {
      Parameter p(pIter->type, pIter->value);
      //p.name() = pIter->name;
      func.parameters().push_back(p);
    }

    if(single)
      return &func;
    else
      top.actions().push_back(func);
  }

  return &top;
}

Action::Ptr ActionNGXVisitor::operator()(smoc_func_diverge& f) const {
  return NULL;
}

Action::Ptr ActionNGXVisitor::operator()(smoc_sr_func_pair& f) const {
  return NULL;
}
#endif // SYSTEMOC_ENABLE_SGX
