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

#include <CoSupport/String/Concat.hpp>
#include <CoSupport/sassert.h>

#include <systemoc/smoc_config.h>

#include <smoc/detail/GraphBase.hpp>
#include <smoc/detail/DebugOStream.hpp>

namespace smoc { namespace Detail {

using CoSupport::String::Concat;

GraphBase::GraphBase(
    const sc_core::sc_module_name &name, smoc_state *init)
  : NodeBase(name, NodeBase::NODE_TYPE_GRAPH, init, 0),
    scheduler(nullptr)
{}
  
void GraphBase::before_end_of_elaboration() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<GraphBase::before_end_of_elaboration name=\"" << name() << "\">"
      << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  if (scheduler)
    scheduler->_before_end_of_elaboration();

  NodeBase::before_end_of_elaboration();
  
  for (std::vector<sc_core::sc_object *>::const_iterator iter = get_child_objects().begin();
       iter != get_child_objects().end();
       ++iter) {
    sassert(childLookupMap.insert(std::make_pair((*iter)->name(), *iter)).second);
    // only processing children which are nodes
    NodeBase *node = dynamic_cast<NodeBase*>(*iter);
    if (node)
      nodes.push_back(node);
    // only processing children which are channels
    ChanBase *channel = dynamic_cast<ChanBase*>(*iter);
    if (channel)
      channels.push_back(channel);
  }

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</GraphBase::before_end_of_elaboration>" << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

void GraphBase::end_of_elaboration() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<GraphBase::end_of_elaboration name=\"" << name() << "\">"
           << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  if (scheduler)
    scheduler->_end_of_elaboration();

  NodeBase::end_of_elaboration();

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</GraphBase::end_of_elaboration>" << std::endl;
  }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
}

const NodeList &GraphBase::getNodes() const
  { return nodes; }

const smoc_chan_list &GraphBase::getChans() const
  { return channels; }

sc_core::sc_object   *GraphBase::getChild(std::string const &name) const {
  std::map<std::string, sc_core::sc_object *>::const_iterator iter =
      childLookupMap.find(name);
  return iter != childLookupMap.end()
      ? iter->second
      : nullptr;
}

void GraphBase::doReset() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<GraphBase::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG

  // Reset all FIFOs.
  for(smoc_chan_list::iterator iter = channels.begin();
      iter != channels.end();
      ++iter)
    (*iter)->doReset();
  // Reset all actors and subgraphs.
  for(NodeList::iterator iter = nodes.begin();
      iter != nodes.end();
      ++iter)
    (*iter)->doReset();
  // Finally, reset myself.
  NodeBase::doReset();
  
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</GraphBase::doReset>" << std::endl;
  }
#endif //SYSTEMOC_ENABLE_DEBUG
}

void GraphBase::setScheduler(smoc_scheduler_top *scheduler) {
  assert(!this->scheduler); assert(scheduler);
  this->scheduler = scheduler;
}
  
} } // namespace smoc::Detail
