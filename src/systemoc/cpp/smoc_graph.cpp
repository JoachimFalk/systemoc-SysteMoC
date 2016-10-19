//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/String/Concat.hpp>

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_graph.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <systemoc/detail/smoc_sysc_port.hpp>
#include <smoc/detail/DebugOStream.hpp>

using CoSupport::String::Concat;

smoc_graph::smoc_graph(const sc_core::sc_module_name &name)
  : smoc::Detail::GraphBase(name, run)
  , run("run")
{
#ifdef SYSTEMOC_ENABLE_MAESTRO
  this->setName(this->name());
#endif //SYSTEMOC_ENABLE_MAESTRO
  constructor();
}

smoc_graph::smoc_graph()
  : smoc::Detail::GraphBase(sc_core::sc_gen_unique_name("smoc_graph"), run)
  , run("run")
{
  constructor();
#ifdef SYSTEMOC_ENABLE_MAESTRO
  this->setName(this->name());
#endif //SYSTEMOC_ENABLE_MAESTRO
}
  
void smoc_graph::before_end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_graph::before_end_of_elaboration name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG
  
  smoc::Detail::GraphBase::before_end_of_elaboration();
  initDDF();

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_graph::before_end_of_elaboration>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}

void smoc_graph::constructor() {

  // if there is at least one active transition: execute it
  run = smoc::Expr::till(ol) >> SMOC_CALL(smoc_graph::scheduleDDF) >> run;
}

void smoc_graph::initDDF() {
  // FIXME if this an initial transition, ol must be cleared
  // up to now, this is called in before_end_of_elaboration...
  for (smoc_node_list::const_iterator iter = getNodes().begin();
       iter != getNodes().end();
       ++iter)
    ol |= **iter;
}

void smoc_graph::scheduleDDF() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_graph::scheduleDDF name=\"" << name() << "\">"
           << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG

  while(ol) {
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << ol << std::endl;
    }
#endif // SYSTEMOC_DEBUG
    smoc_root_node &n = ol.getEventTrigger();
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << "<node name=\"" << n.name() << "\">" << std::endl
             << smoc::Detail::Indent::Up;
    }
#endif // SYSTEMOC_DEBUG
    n.schedule();
#ifdef SYSTEMOC_DEBUG
    if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
      smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</node>" << std::endl;
    }
#endif // SYSTEMOC_DEBUG
  }

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_graph::scheduleDDF>" << std::endl;
  }
#endif // SYSTEMOC_DEBUG
}
