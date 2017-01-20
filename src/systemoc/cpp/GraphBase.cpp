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

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/String/Concat.hpp>

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <systemoc/detail/smoc_sysc_port.hpp>
#include <smoc/detail/GraphBase.hpp>
#include <smoc/detail/DebugOStream.hpp>

namespace smoc { namespace Detail {

using CoSupport::String::Concat;

GraphBase::GraphBase(
    const sc_core::sc_module_name &name, smoc_firing_state &init)
  : smoc_root_node(name, smoc_root_node::NODE_TYPE_GRAPH, init),
#ifdef SYSTEMOC_ENABLE_MAESTRO
	MetaMap::SMoCGraph(name.operator const char *()),
#endif //SYSTEMOC_ENABLE_MAESTRO
    scheduler(nullptr)
{}
  
const smoc_node_list &GraphBase::getNodes() const
  { return nodes; } 

const smoc_chan_list &GraphBase::getChans() const
  { return channels; }

/*
void GraphBase::getNodesRecursive( smoc_node_list & subnodes) const {
  for (
#if SYSTEMC_VERSION < 20050714
        sc_core::sc_pvector<sc_core::sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_core::sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    //smoc::Detail::outDbg << (*iter)->name() << std::endl;
    
    smoc_root_node *node = dynamic_cast<smoc_actor *>(*iter);
    if (node != nullptr){
      //smoc::Detail::outDbg << "add: " <<  node->name() << std::endl;
      subnodes.push_back(node);
    }
    GraphBase *graph = dynamic_cast<GraphBase *>(*iter);
    if (graph != nullptr){
      //smoc::Detail::outDbg << "sub_graph: " <<  graph->name() << std::endl;
      graph->getNodesRecursive(subnodes);
    }
  }
  //  return subnodes;
}
 */

/*
void GraphBase::getChansRecursive( smoc_chan_list & channels) const {

  for (
#if SYSTEMC_VERSION < 20050714
        sc_core::sc_pvector<sc_core::sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_core::sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    smoc_root_chan *chan = dynamic_cast<smoc_root_chan *>(*iter);
    
    if (chan != nullptr )
      channels.push_back(chan);

    GraphBase *graph = dynamic_cast<GraphBase *>(*iter);
    if (graph != nullptr ){
      graph->getChansRecursive( channels );
    }
  }
}
 */

void GraphBase::before_end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<GraphBase::before_end_of_elaboration name=\"" << name() << "\">"
      << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  if (scheduler)
    scheduler->_before_end_of_elaboration();

  smoc_root_node::before_end_of_elaboration();
  
  for (std::vector<sc_core::sc_object *>::const_iterator iter = get_child_objects().begin();
       iter != get_child_objects().end();
       ++iter) {
    // only processing children which are nodes
    smoc_root_node *node = dynamic_cast<smoc_root_node*>(*iter);
    if (node)
      nodes.push_back(node);
  }
  
  for (std::vector<sc_core::sc_object *>::const_iterator iter = get_child_objects().begin();
       iter != get_child_objects().end();
       ++iter)
  {
    // only processing children which are channels
    smoc_root_chan *channel = dynamic_cast<smoc_root_chan*>(*iter);
    if (channel)
      channels.push_back(channel);
  }

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</GraphBase::before_end_of_elaboration>" << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

void GraphBase::end_of_elaboration() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<GraphBase::end_of_elaboration name=\"" << name() << "\">"
           << std::endl << smoc::Detail::Indent::Up;
  }
#endif //defined(SYSTEMOC_DEBUG)
  if (scheduler)
    scheduler->_end_of_elaboration();

  smoc_root_node::end_of_elaboration();

#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</GraphBase::end_of_elaboration>" << std::endl;
  }
#endif //defined(SYSTEMOC_DEBUG)
}

void GraphBase::doReset() {
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<GraphBase::doReset name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_DEBUG

  // Reset all FIFOs.
  for(smoc_chan_list::iterator iter = channels.begin();
      iter != channels.end();
      ++iter)
    (*iter)->doReset();
  // Reset all actors and subgraphs.
  for(smoc_node_list::iterator iter = nodes.begin();
      iter != nodes.end();
      ++iter)
    (*iter)->doReset();
  // Finally, reset myself.
  smoc_root_node::doReset();
  
#ifdef SYSTEMOC_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</GraphBase::doReset>" << std::endl;
  }
#endif //SYSTEMOC_DEBUG
}

void GraphBase::setScheduler(smoc_scheduler_top *scheduler) {
  assert(!this->scheduler); assert(scheduler);
  this->scheduler = scheduler;
}
  
} } // namespace smoc::Detail
