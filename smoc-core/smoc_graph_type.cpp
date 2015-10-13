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

#include <systemoc/smoc_config.h>

#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_firing_rules.hpp>
#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/detail/smoc_debug_stream.hpp>

#include <CoSupport/compatibility-glue/nullptr.h>

#include <CoSupport/String/Concat.hpp>

using namespace smoc::Detail;
using CoSupport::String::Concat;

smoc_graph_base::smoc_graph_base(
  const sc_module_name &name, smoc_firing_state &init)
: smoc_root_node(name, smoc_root_node::NODE_TYPE_GRAPH, init) {}
  
const smoc_node_list& smoc_graph_base::getNodes() const
  { return nodes; } 

void smoc_graph_base::getNodesRecursive( smoc_node_list & subnodes) const {
  for (
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    //outDbg << (*iter)->name() << std::endl;
    
    smoc_root_node *node = dynamic_cast<smoc_actor *>(*iter);
    if (node != nullptr){
      //outDbg << "add: " <<  node->name() << std::endl;
      subnodes.push_back(node);
    }
    smoc_graph_base *graph = dynamic_cast<smoc_graph_base *>(*iter);
    if (graph != nullptr){
      //outDbg << "sub_graph: " <<  graph->name() << std::endl;
      graph->getNodesRecursive(subnodes);
    }
  }
  //  return subnodes;
}
 
const smoc_chan_list &smoc_graph_base::getChans() const
  { return channels; }

void smoc_graph_base::getChansRecursive( smoc_chan_list & channels) const {

  for (
#if SYSTEMC_VERSION < 20050714
        sc_pvector<sc_object*>::const_iterator iter = get_child_objects().begin();
#else
        std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
#endif
        iter != get_child_objects().end();
        ++iter ) {
    smoc_root_chan *chan = dynamic_cast<smoc_root_chan *>(*iter);
    
    if (chan != nullptr )
      channels.push_back(chan);

    smoc_graph_base *graph = dynamic_cast<smoc_graph_base *>(*iter);
    if (graph != nullptr ){
      graph->getChansRecursive( channels );
    }
  }
}

void smoc_graph_base::finalise() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_graph_base::finalise name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
  
  smoc_root_node::finalise();
  
  // FIXME: Sync. WILL have to be different than now
  
  // SystemC --> SGX
  for (std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
       iter != get_child_objects().end();
       ++iter)
  {
    // only processing children which are nodes
    smoc_root_node* node = dynamic_cast<smoc_root_node*>(*iter);
    if(!node) continue;
    
/*  // determine if node is in XML; otherwise it will be "hidden"
    if(NGXConfig::getInstance().hasNGX()) {  
      
      Process::ConstPtr proc =
        objAs<Process>(NGXCache::getInstance().get(node));

      if(!proc || proc->owner()->id() != idGraph)
        continue;
    }*/

    nodes.push_back(node);
  }
  
  for(std::vector<sc_object*>::const_iterator iter = get_child_objects().begin();
      iter != get_child_objects().end();
      ++iter)
  {
    // only processing children which are channels
    smoc_root_chan* channel = dynamic_cast<smoc_root_chan*>(*iter);
    if(!channel) continue;
    
    channels.push_back(channel);
  }

  // finalise for actors must precede finalise for channels,
  // because finalise for channels needs the patched in actor
  // references in the ports which are updated by the finalise
  // methods of their respective actors
  // FIXME: seems no longer to be done this way
  for(smoc_node_list::iterator iter = nodes.begin();
      iter != nodes.end();
      ++iter)
  {
    (*iter)->finalise();
  }
  
  for(smoc_chan_list::iterator iter = channels.begin();
      iter != channels.end();
      ++iter)
  {
    (*iter)->finalise();
  }
  
//#ifdef SYSTEMOC_ENABLE_SGX
//  // FIXME: FSM is attribute of Actor, not of Process
//  pg->firingFSM() = getFiringFSM()->getNGXObj();
//#endif

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_graph_base::finalise>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

#ifdef SYSTEMOC_ENABLE_VPC
void smoc_graph_base::finaliseVpcLink() {
  for(smoc_node_list::iterator iter = nodes.begin();
      iter != nodes.end();
      ++iter)
  {
    (*iter)->finaliseVpcLink();
  }
}
#endif //SYSTEMOC_ENABLE_VPC


void smoc_graph_base::doReset() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_graph_base::doReset name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

  // reset FIFOs
  for(smoc_chan_list::iterator iter = channels.begin();
      iter != channels.end();
      ++iter)
  {
    (*iter)->doReset();
  }

  // reset child nodes
  for(smoc_node_list::iterator iter = nodes.begin();
      iter != nodes.end();
      ++iter)
  {
    (*iter)->doReset();
  }

  smoc_root_node::doReset();

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_graph_base::doReset>" << std::endl;
#endif //SYSTEMOC_DEBUG
}
  
smoc_graph::smoc_graph(const sc_module_name& name) :
  smoc_graph_base(name, run),
  run("run")
#ifdef SYSTEMOC_ENABLE_MAESTROMM
  , SMoCGraph(name)
#endif
{
#ifdef SYSTEMOC_ENABLE_MAESTROMM
	this->setName(this->name());
#endif

  constructor();
}

smoc_graph::smoc_graph() :
  smoc_graph_base(sc_gen_unique_name("smoc_graph"), run),
  run("run")
{
  constructor();

#ifdef SYSTEMOC_ENABLE_MAESTROMM
  this->setName(this->name());
#endif
}
  
void smoc_graph::finalise() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_graph::finalise name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG
  
  smoc_graph_base::finalise();
  initDDF();

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_graph::finalise>" << std::endl;
#endif // SYSTEMOC_DEBUG
}

void smoc_graph::constructor() {

  // if there is at least one active transition: execute it
  run = smoc::Expr::till(ol) >> CALL(smoc_graph::scheduleDDF) >> run;
}

void smoc_graph::initDDF() {
  // FIXME if this an initial transition, ol must be cleared
  // up to now, this is called in finalise...
  for(smoc_node_list::const_iterator iter = getNodes().begin();
      iter != getNodes().end(); ++iter)
  {
    ol |= **iter;
  }
}

void smoc_graph::scheduleDDF() {
#ifdef SYSTEMOC_DEBUG
  outDbg << "<smoc_graph::scheduleDDF name=\"" << name() << "\">"
         << std::endl << Indent::Up;
#endif // SYSTEMOC_DEBUG

  while(ol) {
#ifdef SYSTEMOC_DEBUG
    outDbg << ol << std::endl;
#endif // SYSTEMOC_DEBUG
    smoc_root_node &n = ol.getEventTrigger();
#ifdef SYSTEMOC_DEBUG
    outDbg << "<node name=\"" << n.name() << "\">" << std::endl
           << Indent::Up;
#endif // SYSTEMOC_DEBUG
    n.schedule();
#ifdef SYSTEMOC_DEBUG
    outDbg << Indent::Down << "</node>" << std::endl;
#endif // SYSTEMOC_DEBUG
  }

#ifdef SYSTEMOC_DEBUG
  outDbg << Indent::Down << "</smoc_graph::scheduleDDF>" << std::endl;
#endif // SYSTEMOC_DEBUG
}
