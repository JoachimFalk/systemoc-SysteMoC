//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of
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

#ifdef SYSTEMOC_ENABLE_SGX

#include <sgx.hpp>
#include <CoSupport/String/Concat.hpp>

#include <smoc/detail/apply_visitor.hpp>

namespace SysteMoC { namespace Detail {

using CoSupport::String::Concat;

template <class Visitor>
void recurse(Visitor &visitor, sc_object &obj) {
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
    apply_visitor(visitor, **iter);
  }
}

class ProcessSubVisitor {
public:
  typedef void result_type;
protected:
  SGX::Process &proc;
protected:
  SGX::Process &getProcess()
    { return proc; }
public:
  ProcessSubVisitor(SGX::Process &proc)
    : proc(proc) {}

  void operator ()(smoc_sysc_port &obj);

  void operator ()(sc_object &obj)
    { /* ignore */ }
};

class GraphSubVisitor {
public:
  typedef void result_type;
protected:
  SGX::ProblemGraph &pg;
public:
  GraphSubVisitor(SGX::ProblemGraph &pg)
    : pg(pg) {}

//  void operator ()(smoc_sysc_port &obj) {
//    proc.ports().push_back(*DumpPort()(obj));
//  }

  void operator ()(smoc_graph_base &obj);

  void operator ()(smoc_actor &obj);

  void operator ()(smoc_fifo_chan_base &obj);

//  void operator ()(smoc_multiplex_fifo_chan_bases &obj) {
//    pg.processes().push_back(*DumpMultiplexFifo()(obj));
//  }

//  void operator ()(smoc_multireader_fifo_chan_base &obj) {
//    pg.processes().push_back(*DumpMultireaderFifo()(obj));
//  }

  void operator ()(sc_object &obj)
    { /* ignore */ }
};

class ActorSubVisitor: public ProcessSubVisitor {
public:
  typedef void result_type;
protected:
  SGX::Actor &getActor()
    { return static_cast<SGX::Actor &>(proc); }
public:
  ActorSubVisitor(SGX::Actor &actor)
    : ProcessSubVisitor(actor) {}

  using ProcessSubVisitor::operator();
};

class DumpPort {
public:
  typedef SGX::Port::Ptr result_type;
public:
  result_type operator ()(smoc_sysc_port &p) {
    SGX::Port port(p.name(), p.getId());
    return &port;
  }
};

class DumpFifo {
public:
  typedef SGX::Fifo::Ptr result_type;
public:

  void foo(SGX::Channel &channel, smoc_root_chan &rc) {
    for (smoc_root_chan::EntryMap::const_iterator iter = rc.getEntries().begin();
         iter != rc.getEntries().end();
         ++iter) {
      SGX::Port p(Concat(rc.name())(".in"));
      p.direction() = SGX::Port::IN;
      channel.ports().push_back(p);
    }
    for (smoc_root_chan::OutletMap::const_iterator iter = rc.getOutlets().begin();
         iter != rc.getOutlets().end();
         ++iter) {
      SGX::Port p(Concat(rc.name())(".out"));
      p.direction() = SGX::Port::OUT;
      channel.ports().push_back(p);
    }
  }

  result_type operator ()(smoc_fifo_chan_base &p) {
    SGX::Fifo fifo(p.name(), p.getId());
    foo(fifo, p);
    return &fifo;
  }
};

class DumpActor {
public:
  typedef SGX::Actor::Ptr result_type;
protected:

public:
  result_type operator ()(smoc_actor &a) {
    SGX::Actor actor(a.name(), a.getId());
    ActorSubVisitor sv(actor);
    recurse(sv, a);
    return &actor;
  }
};

class DumpGraph {
public:
  typedef SGX::RefinedProcess::Ptr result_type;
protected:

public:
  result_type operator ()(smoc_graph_base &g) {
    SGX::RefinedProcess rp(Concat(g.name())("_rp"));
    SGX::ProblemGraph   pg(g.name(), g.getId());
    rp.refinements().push_back(pg);
    GraphSubVisitor sv(pg);
    recurse(sv, g);
    return &rp;
  }


};

void ProcessSubVisitor::operator ()(smoc_sysc_port &obj) {
  proc.ports().push_back(*DumpPort()(obj));
}

void GraphSubVisitor::operator ()(smoc_graph_base &obj) {
  pg.processes().push_back(*DumpGraph()(obj));
}

void GraphSubVisitor::operator ()(smoc_actor &obj) {
  pg.processes().push_back(*DumpActor()(obj));
}

void GraphSubVisitor::operator ()(smoc_fifo_chan_base &obj) {
  pg.processes().push_back(*DumpFifo()(obj));
}

/*
  template <typename T>
  result_type process(T &obj, sc_object &) {
    std::cerr << typeid(T).name() << ": " << obj.name() << std::endl;
    recurse(*this, obj);
  }

  template <typename T>
  result_type operator ()(T &obj)
    { this->process(obj, obj); }
 */

SGX::RefinedProcess::Ptr dumpSMX(smoc_graph_base &g) {
  return DumpGraph()(g);
}

} } // namespace SysteMoC::Detail

#endif // SYSTEMOC_ENABLE_SGX
