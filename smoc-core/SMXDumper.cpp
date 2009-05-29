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

#include "apply_visitor.hpp"

#include <map>
#include <utility>

namespace SysteMoC { namespace Detail {

typedef std::map<sc_port_base *, SGX::Port::Ptr>  SCPortBase2ActorPort;

struct SMXDumpCTX {
  SCPortBase2ActorPort scPortBase2ActorPort;
};

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
  SMXDumpCTX   &ctx;
  SGX::Process &proc;
protected:
  SGX::Process &getProcess()
    { return proc; }
public:
  ProcessSubVisitor(SMXDumpCTX &ctx, SGX::Process &proc)
    : ctx(ctx), proc(proc) {}

  void operator ()(smoc_sysc_port &obj);

  void operator ()(sc_object &obj)
    { /* ignore */ }
};

class GraphSubVisitor {
public:
  typedef void result_type;
protected:
  SMXDumpCTX        &ctx;
  SGX::ProblemGraph &pg;
public:
  GraphSubVisitor(SMXDumpCTX &ctx, SGX::ProblemGraph &pg)
    : ctx(ctx), pg(pg) {}

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
  ActorSubVisitor(SMXDumpCTX &ctx, SGX::Actor &actor)
    : ProcessSubVisitor(ctx, actor) {}

  using ProcessSubVisitor::operator();
};

class DumpPort {
public:
  typedef void result_type;
protected:
  SMXDumpCTX   &ctx;
  SGX::Process &proc;
public:
  DumpPort(SMXDumpCTX &ctx, SGX::Process &proc)
    : ctx(ctx), proc(proc) {}

  result_type operator ()(smoc_sysc_port &p) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "DumpPort::operator ()(...) [BEGIN]" << std::endl;
#endif
    SGX::Port port(p.name(), p.getId());
    sassert(ctx.scPortBase2ActorPort.insert(std::make_pair(&p, &port)).second);
    proc.ports().push_back(port);
    port.direction() = p.isInput() ? SGX::Port::In : SGX::Port::Out;
#ifdef SYSTEMOC_DEBUG
    std::cerr << "DumpPort::operator ()(...) [END]" << std::endl;
#endif
  }
};

class DumpFifo {
public:
  typedef void result_type;
protected:
  SMXDumpCTX        &ctx;
  SGX::ProblemGraph &pg;
public:
  DumpFifo(SMXDumpCTX &ctx, SGX::ProblemGraph &pg)
    : ctx(ctx), pg(pg) {}

  void connectPort(SGX::Port &pChan, sc_port_base *pActor) {
    SCPortBase2ActorPort::iterator iter =
      ctx.scPortBase2ActorPort.find(pActor);
    if (iter != ctx.scPortBase2ActorPort.end())
      pChan.peerPort() = iter->second;
    else
      std::cerr << "oops!!!" << std::endl;
  }

  void foo(SGX::Channel &channel, smoc_root_chan &rc) {
    for (smoc_root_chan::EntryMap::const_iterator iter = rc.getEntries().begin();
         iter != rc.getEntries().end();
         ++iter) {
      SGX::Port p(Concat(rc.name())(".in"));
      p.direction() = SGX::Port::In;
      channel.ports().push_back(p);
      connectPort(p, iter->second);
    }
    for (smoc_root_chan::OutletMap::const_iterator iter = rc.getOutlets().begin();
         iter != rc.getOutlets().end();
         ++iter) {
      SGX::Port p(Concat(rc.name())(".out"));
      p.direction() = SGX::Port::Out;
      channel.ports().push_back(p);
      connectPort(p, iter->second);
    }
  }

  result_type operator ()(smoc_fifo_chan_base &p) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "DumpFifo::operator ()(...) [BEGIN]" << std::endl;
#endif
    SGX::Fifo fifo(p.name(), p.getId());
    pg.processes().push_back(fifo);
    foo(fifo, p);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "DumpFifo::operator ()(...) [END]" << std::endl;
#endif
  }
};

class DumpActor {
public:
  typedef void result_type;
protected:
  SMXDumpCTX        &ctx;
  SGX::ProblemGraph &pg;
public:
  DumpActor(SMXDumpCTX &ctx, SGX::ProblemGraph &pg)
    : ctx(ctx), pg(pg) {}

  result_type operator ()(smoc_actor &a) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "DumpActor::operator ()(...) [BEGIN]" << std::endl;
#endif
    SGX::Actor actor(a.name(), a.getId());
    pg.processes().push_back(actor);
    ActorSubVisitor sv(ctx, actor);
    recurse(sv, a);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "DumpActor::operator ()(...) [END]" << std::endl;
#endif
  }
};

class DumpGraph {
public:
  typedef void result_type;
protected:
  SMXDumpCTX        &ctx;
  SGX::ProblemGraph &pg;
public:
  DumpGraph(SMXDumpCTX &ctx, SGX::ProblemGraph &pg)
    : ctx(ctx), pg(pg) {}

  result_type operator ()(smoc_graph_base &g) {
#ifdef SYSTEMOC_DEBUG
    std::cerr << "DumpGraph::operator ()(...) [BEGIN]" << std::endl;
#endif
    SGX::RefinedProcess rp(Concat(g.name())("_rp"));
    pg.processes().push_back(rp);
    SGX::ProblemGraph   pg(g.name(), g.getId());
    rp.refinements().push_back(pg);
    GraphSubVisitor sv(ctx,pg);
    recurse(sv, g);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "DumpGraph::operator ()(...) [END]" << std::endl;
#endif
  }
};

void ProcessSubVisitor::operator ()(smoc_sysc_port &obj) {
  DumpPort(ctx, proc)(obj);
}

void GraphSubVisitor::operator ()(smoc_graph_base &obj) {
  DumpGraph(ctx, pg)(obj);
}

void GraphSubVisitor::operator ()(smoc_actor &obj) {
  DumpActor(ctx, pg)(obj);
}

void GraphSubVisitor::operator ()(smoc_fifo_chan_base &obj) {
  DumpFifo(ctx, pg)(obj);
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

void dumpSMX(std::ostream &file, smoc_graph_base &g) {
  SGX::NetworkGraphAccess ngx;
  SMXDumpCTX              ctx;
  SGX::ProblemGraph       pg(g.name(), g.getId());
  GraphSubVisitor         sv(ctx,pg);
  
  recurse(sv, g);
  ngx.problemGraphPtr() = &pg;
  ngx.architectureGraphPtr() = SGX::ArchitectureGraph("dummy architecture graph").toPtr(); 
  ngx.save(file);
}

} } // namespace SysteMoC::Detail

#endif // SYSTEMOC_ENABLE_SGX
