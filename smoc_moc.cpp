//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_graph_type.hpp>
#include <systemoc/smoc_sr_signal.hpp>
#include <systemoc/smoc_multicast_sr_signal.hpp>
#include <smoc/smoc_simulation_ctx.hpp>
#include <smoc/detail/apply_visitor.hpp>

#include <CoSupport/DataTypes/oneof.hpp>

#ifdef SYSTEMOC_DEBUG
# define DEBUG_CODE(code) code
#else
# define DEBUG_CODE(code) do {} while(0);
#endif

using namespace SystemCoDesigner::SGX;
using namespace CoSupport;

smoc_scheduler_top::smoc_scheduler_top(smoc_graph_base* g) :
  sc_module(sc_module_name("smoc_scheduler_top")),
  g(g),
  simulation_running(false)
{
  SC_THREAD(schedule);
}

smoc_scheduler_top::smoc_scheduler_top(smoc_graph_base& g) :
  sc_module(sc_module_name("smoc_scheduler_top")),
  g(&g),
  simulation_running(false)
{
  SC_THREAD(schedule);
}

smoc_scheduler_top::~smoc_scheduler_top() {
  if(simulation_running)
    sc_core::sc_stop();
}

void smoc_scheduler_top::start_of_simulation()
{ simulation_running = true; }

void smoc_scheduler_top::end_of_simulation() {
  simulation_running = false;
#ifdef SYSTEMOC_ENABLE_SGX
  if (getSimCTX()->isSMXDumpingPostSimEnabled()) {
    assert(!"At the moment unsupported!");
  }
#endif // SYSTEMOC_ENABLE_SGX
}

template <class DERIVED>
class RecurseVisitorBase {
  typedef RecurseVisitorBase<DERIVED> this_type;
protected:
  DERIVED       *derived()
    { return static_cast<DERIVED       *>(this); }
  DERIVED const *derived() const
    { return static_cast<DERIVED const *>(this); }
protected:

  void recurse(sc_object &obj) {
#if SYSTEMC_VERSION < 20050714
    typedef sc_pvector<sc_object*> sc_object_list;
#else
    typedef std::vector<sc_object*>  sc_object_list;
#endif
    for (sc_object_list::const_iterator iter = obj.get_child_objects().begin();
         iter != obj.get_child_objects().end();
         ++iter ) {
//    if (dynamic_cast<smoc_root_node *>(*iter))
//      apply_visitor(*this, *static_cast<smoc_root_node *>(*iter));
      apply_visitor(*derived(), **iter);
    }
  }
};

class Dumping
: public RecurseVisitorBase<Dumping> {
  typedef Dumping this_type;
public:
  typedef void result_type;

  template <typename T>
  result_type process(T &obj, sc_object &) {
    std::cerr << typeid(T).name() << ": " << obj.name() << std::endl;
    recurse(obj);
  }

  template <typename T>
  result_type operator ()(T &obj)
    { this->process(obj, obj); }
};

void smoc_scheduler_top::end_of_elaboration() {
  g->finalise();
  g->reset();
#ifdef SYSTEMOC_ENABLE_SGX
  Dumping d;

  apply_visitor(d, *static_cast<smoc_root_node *>(g));

  if (getSimCTX()->isSMXDumpingPreSimEnabled()) {
    ArchitectureGraph ag("architecture graph");
    getSimCTX()->getExportNGX().architectureGraphPtr() = &ag;
    getSimCTX()->getExportNGX().save(getSimCTX()->getSMXPreSimFile());
    sc_core::sc_stop();
  }
#endif // SYSTEMOC_ENABLE_SGX
}
  
void smoc_scheduler_top::schedule() {
  smoc_transition_ready_list ol;
  
  // add outgoing transitions to list
  g->addCurOutTransitions(ol);
  
  while(true) {
    smoc_wait(ol);
    while(ol) {
#ifdef SYSTEMOC_DEBUG
      std::cerr << ol << std::endl;
#endif
      RuntimeTransition &transition = ol.getEventTrigger();
      // We have waited on a transition so it should no longer be blocked
      assert(transition);
      // It should either be enabled so we can execute it or its functionallity
      // condition could disable it.
      Expr::Detail::ActivationStatus status = transition.getStatus();
      
      switch(status.toSymbol()) {
        case Expr::Detail::_DISABLED:
          // remove disabled transition
          assert(&transition.getActor() == g);
          ol.remove(transition);
          break;
        case Expr::Detail::_ENABLED:
          // execute enabled transition
          assert(&transition.getActor() == g);
#ifdef SYSTEMOC_DEBUG
          std::cerr << "<node name=\"" << g->name() << "\">" << std::endl;
#endif
          // remove transitions from list
          g->delCurOutTransitions(ol);
          // execute transition
          transition.execute();
          // add transitions to list
          g->addCurOutTransitions(ol);
#ifdef SYSTEMOC_DEBUG
          std::cerr << "</node>" << std::endl;
#endif
          break;
        default:
          assert(!"WTF?! transition not either enabled or disabled!");
      }
    }
  }
}

