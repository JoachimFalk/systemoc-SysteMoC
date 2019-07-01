// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 * 
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <smoc/smoc_action.hpp>
#include <smoc/detail/DebugOStream.hpp>

#include <systemoc/smoc_config.h>

namespace smoc {

void smoc_action::operator()() const {
  // Function call
  for (value_type const &f : *this) {
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    this->getSimCTX()->getDataflowTraceLog()->traceStartFunction(&f);
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "<action type=\"smoc_func_call\" func=\""
           << f->getFuncName() << "\">" << std::endl;
    }
#endif // SYSTEMOC_ENABLE_DEBUG
    f->call();
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::Medium)) {
      smoc::Detail::outDbg << "</action>" << std::endl;
    }
#endif // SYSTEMOC_ENABLE_DEBUG
#ifdef SYSTEMOC_ENABLE_DATAFLOW_TRACE
    getSimCTX()->getDataflowTraceLog()->traceEndFunction(&*i);
#endif // SYSTEMOC_ENABLE_DATAFLOW_TRACE
  }
}

smoc_action Detail::merge(const smoc_action &a, const smoc_action &b) {
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

} // namespace smoc
