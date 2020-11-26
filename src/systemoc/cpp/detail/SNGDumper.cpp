// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#include <systemoc/smoc_config.h>

#include <smoc/detail/DebugOStream.hpp>
#include <smoc/detail/DumpingInterfaces.hpp>
#include <smoc/detail/NamedIdedObj.hpp>
#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/ChanBase.hpp>
#include <smoc/detail/PortBase.hpp>

#include <smoc/smoc_guard.hpp>
#include <smoc/smoc_actor.hpp>

#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_multiplex_fifo.hpp>
#include <systemoc/smoc_multireader_fifo.hpp>

#include "apply_visitor.hpp"
#include "SimulationContext.hpp"

//#include "FSM/RuntimeState.hpp"
//#include "FSM/RuntimeFiringRule.hpp"
//#include "FSM/RuntimeTransition.hpp"
//#include "FSM/FiringFSM.hpp"

#include <CoSupport/String/Concat.hpp>
#include <CoSupport/String/convert.hpp>
#include <CoSupport/String/DoubleQuotedString.hpp>
#include <CoSupport/String/UniquePool.hpp>

#include <map>
#include <utility>
#include <memory>
#include <cstdlib>

#ifdef __GNUG__
# include <cxxabi.h>
#endif

/*

<?xml version="1.0"?>
<networkGraph
  xmlns:xsi="http://www.w3.org/2001/XMLSchema-instance"
  xsi:noNamespaceSchemaLocation="sng.xsd">

  <actorType name="A1">
    <port name="i1" type="in"  rate="1"/>
    <port name="i2" type="in"  rate="1"/>
    <port name="o1" type="out" rate="1"/>
    <port name="o2" type="out" rate="1"/>
  </actorType>
  <actorType name="A2">
    <port name="i1" type="in"  rate="1"/>
    <port name="i2" type="in"  rate="1"/>
    <port name="o1" type="out" rate="1"/>
    <port name="o2" type="out" rate="1"/>
  </actorType>
  <actorType name="A3">
    <port name="i1" type="in"  rate="1"/>
    <port name="i2" type="in"  rate="1"/>
    <port name="o1" type="out" rate="1"/>
    <port name="o2" type="out" rate="1"/>
    <port name="o3" type="out" rate="1"/>
  </actorType>
  <actorType name="A4">
    <port name="i1" type="in"  rate="1"/>
    <port name="o1" type="out" rate="1"/>
    <port name="o2" type="out" rate="1"/>
  </actorType>
  <actorType name="A5">
    <port name="i1" type="in"  rate="1"/>
    <port name="i2" type="in"  rate="1"/>
    <port name="o1" type="out" rate="1"/>
  </actorType>
  <actorType name="A6">
    <port name="i1" type="in"  rate="1"/>
    <port name="i2" type="in"  rate="1"/>
    <port name="i3" type="in"  rate="1"/>
    <port name="o1" type="out" rate="1"/>
  </actorType>

  <actorInstance name="a1" type="A1"/>
  <actorInstance name="a2" type="A2"/>
  <actorInstance name="a3" type="A3"/>
  <actorInstance name="a4" type="A4"/>
  <actorInstance name="a5" type="A5"/>
  <actorInstance name="a6" type="A6"/>

  <fifo size="3" initial="1">
    <source actor="a1" port="o2"/>
    <target actor="a2" port="i2"/>
  </fifo>
  <fifo size="3" initial="1">
    <source actor="a2" port="o2"/>
    <target actor="a1" port="i2"/>
  </fifo>
  <fifo size="3" initial="0">
    <source actor="a1" port="o1"/>
    <target actor="a3" port="i1"/>
  </fifo>
  <fifo size="3" initial="0">
    <source actor="a2" port="o1"/>
    <target actor="a3" port="i2"/>
  </fifo>
  <fifo size="3" initial="0">
    <source actor="a3" port="o1"/>
    <target actor="a4" port="i1"/>
  </fifo>
  <fifo size="3" initial="0">
    <source actor="a3" port="o2"/>
    <target actor="a5" port="i1"/>
  </fifo>
  <fifo size="3" initial="0">
    <source actor="a3" port="o3"/>
    <target actor="a6" port="i3"/>
  </fifo>
  <fifo size="3" initial="0">
    <source actor="a4" port="o2"/>
    <target actor="a5" port="i2"/>
  </fifo>
  <fifo size="3" initial="0">
    <source actor="a5" port="o1"/>
    <target actor="a6" port="i2"/>
  </fifo>

</networkGraph>

 */


//#define SYSTEMOC_ENABLE_DEBUG

namespace smoc { namespace Detail {

using CoSupport::String::Concat;
using CoSupport::String::asStr;

typedef CoSupport::String::DoubleQuotedString DQ;

SimulationContextSNGDumping::SimulationContextSNGDumping()
  : dumpSNGFile(nullptr) {}

SimulationContextSNGDumping::~SimulationContextSNGDumping() {
  if (isSNGDumpingEnabled()) {
    dumpSNGFile->flush();
    delete dumpSNGFile;
    dumpSNGFile = nullptr;
  }
}

namespace { // anonymous

std::string demangle(const char *name) {
#ifdef __GNUG__
  int status = -4; // some arbitrary value to eliminate the compiler warning

  std::unique_ptr<char, void(*)(void*)> res {
      abi::__cxa_demangle(name, NULL, NULL, &status),
      std::free
  };

  return (status==0) ? res.get() : name ;
#else
  // does nothing if not g++
  return name;
#endif
}

struct FlummyPort {
  FlummyPort(
      std::string const &actorName
    , std::string const &portName
    , bool isInput)
    : actorName(actorName)
    , portName(portName)
    , isInput(isInput) {}

  std::string actorName;
  std::string portName;
  bool        isInput;
};

typedef std::map<sc_core::sc_port_base const *, FlummyPort>  SCPortBase2Port;
typedef std::map<sc_core::sc_interface *,       FlummyPort>  SCInterface2Port;

struct SNGDumpCTX {
  SimulationContextSNGDumping        *simCTX;
  std::map<std::string, std::string>  actorTypeCache;
  CoSupport::String::UniquePool       actorTypeUniquePool;

  std::stringstream actorTypes;
  std::stringstream actorInstances;
  std::stringstream fifoConnections;

  SNGDumpCTX(SimulationContextSNGDumping *ctx)
    : simCTX(ctx) {}
};

template <class Visitor>
void recurse(Visitor &visitor, sc_core::sc_object &obj) {
#if SYSTEMC_VERSION < 20050714
  typedef sc_core::sc_pvector<sc_core::sc_object*> sc_object_list;
#else
  typedef std::vector<sc_core::sc_object*>         sc_object_list;
#endif
  {
    sc_core::sc_module *mod;
    for (sc_object_list::const_iterator iter = obj.get_child_objects().begin();
         iter != obj.get_child_objects().end();
         ++iter) {
      // Actors/Graphs first!
      if ((mod = dynamic_cast<sc_core::sc_module *>(*iter))) {
        // But ignore internal SysteMoC sc_modules, which should start with __smoc_!
        // -> first identify leaf name of the hierarchical SystemC name, i.e., if name == "a.b.c" then leaf name == "c"
        std::string            name = mod->name();
        std::string::size_type pos  = name.rfind('.');
        if (pos != std::string::npos)
          pos++;
        else
          pos = 0;
        // Now name.substr(pos, ...) is the leaf name
        if (name.substr(pos, sizeof("__smoc_")-1) != "__smoc_")
          apply_visitor(visitor, *static_cast<sc_core::sc_module *>(*iter));
      }
    }
  }
  {
    ChanBase *chan;
    for (sc_object_list::const_iterator iter = obj.get_child_objects().begin();
         iter != obj.get_child_objects().end();
         ++iter) {
      // Channels next!
      if ((chan = dynamic_cast<ChanBase *>(*iter)))
        apply_visitor(visitor, *chan);
    }
  }
  {
    sc_core::sc_port_base *port;
    for (sc_object_list::const_iterator iter = obj.get_child_objects().begin();
         iter != obj.get_child_objects().end();
         ++iter) {
      // Ports last!
      if ((port = dynamic_cast<sc_core::sc_port_base *>(*iter)))
        apply_visitor(visitor, *port);
    }
  }
}

class GraphSubVisitor;

struct ExpectedPortConnections {
  // map from channel entry/outlet to inner port
  SCInterface2Port   unclassifiedPorts;
  // map from outer port to inner port
  SCPortBase2Port    expectedOuterPorts;
  // map from channel entry/outlet to inner port
  SCInterface2Port   expectedChannelConnections;

  ~ExpectedPortConnections() {
    assert(expectedOuterPorts.empty());
    // Disable this till we have covered all channel types
    // assert(expectedChannelConnections.empty());
    for (SCInterface2Port::const_iterator iter = expectedChannelConnections.begin();
         iter != expectedChannelConnections.end();
         ++iter) {
      PortOutBaseIf *entry;
      PortInBaseIf  *outlet;
      
      if ((entry = dynamic_cast<PortOutBaseIf *>(iter->first))) {
        std::cerr << "Unhandled entry type " << typeid(*entry).name()
                  << " => dangling port " << iter->second.portName << std::endl;
      } else if ((outlet = dynamic_cast<PortInBaseIf *>(iter->first))) {
        std::cerr << "Unhandled outlet type " << typeid(*outlet).name()
                  << " => dangling port " << iter->second.portName << std::endl;
      } else {
        std::cerr << "Unhandled entry/outlet type " << typeid(iter->first).name()
                  << " => dangling port " << iter->second.portName << std::endl;
      }
    }
  }
};

class ProcessSubVisitor: public ExpectedPortConnections {
public:
  typedef void result_type;
public:
  SNGDumpCTX              &ctx;
  // one hierarchy up
  ExpectedPortConnections &epc;
  SCPortBase2Port          ports;
public:
  ProcessSubVisitor(SNGDumpCTX &ctx, ExpectedPortConnections &epc)
    : ctx(ctx), epc(epc) {}

  void operator ()(PortBase &obj);

  void operator ()(sc_core::sc_port_base &obj);

  void operator ()(sc_core::sc_object &obj)
    { /* ignore */ }

};

class GraphSubVisitor: public ProcessSubVisitor {
public:
  typedef void result_type;
public:
public:
  GraphSubVisitor(SNGDumpCTX &ctx, ExpectedPortConnections &epc)
    : ProcessSubVisitor(ctx, epc)
    {}

  void operator ()(GraphBase &obj);

  void operator ()(sc_core::sc_module &obj);

  void operator ()(smoc_actor &obj);

  void operator ()(smoc_fifo_chan_base &obj);

  void operator ()(smoc_multireader_fifo_chan_base &obj);

  void operator ()(smoc_multiplex_fifo_chan_base &obj);

  void operator ()(smoc_reset_chan &obj);

  ~GraphSubVisitor();

  using ProcessSubVisitor::operator();
};

class ActorSubVisitor: public ProcessSubVisitor {
public:
  typedef void result_type;
protected:
public:
  ActorSubVisitor(SNGDumpCTX &ctx, ExpectedPortConnections &epc)
    : ProcessSubVisitor(ctx, epc) {}

  using ProcessSubVisitor::operator();
};

class DumpPort: public NamedIdedObjAccess {
public:
  typedef void result_type;
protected:
  ProcessSubVisitor &psv;
public:
  DumpPort(ProcessSubVisitor &psv)
    : psv(psv) {}

  result_type operator ()(PortBase &p) {
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpPort::operator ()(smoc_sysc_port &) [BEGIN]" << std::endl;
    }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    std::string portName = getName(&p);
    std::string::size_type pos = portName.rfind('.');
    assert(pos != std::string::npos && pos > 0);
    std::string actorName = portName.substr(0, pos);
    portName = portName.substr(pos+1);
    FlummyPort port(actorName, portName, p.isInput());
    bool isInput = p.isInput();
    sassert(psv.ports.insert(std::make_pair(&p, port)).second);
    if (p.getActorPort() == &p) {
#ifdef SYSTEMOC_ENABLE_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << getName(&p) << " => expectedChannelConnections";
      }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
      if (isInput) {
        PortInBaseIf *iface = static_cast<PortInBase &>(p).get_interface();
#ifdef SYSTEMOC_ENABLE_DEBUG
        if (outDbg.isVisible(Debug::Low)) {
          outDbg << " " << iface;
        }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
        sassert(psv.epc.expectedChannelConnections.insert(
          std::make_pair(iface, port)).second);
      } else {
        for (PortOutBaseIf *iface : static_cast<PortOutBase &>(p).get_interfaces()) {
#ifdef SYSTEMOC_ENABLE_DEBUG
          if (outDbg.isVisible(Debug::Low)) {
            outDbg << " " << iface;
          }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
          sassert(psv.epc.expectedChannelConnections.insert(
            std::make_pair(iface, port)).second);
        }
      }
#ifdef SYSTEMOC_ENABLE_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << std::endl;
      }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    }
    if (p.getParentPort()) {
#ifdef SYSTEMOC_ENABLE_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << getName(&p) << " => expectedOuterPorts " << p.getParentPort()->name() << std::endl;
      }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
      sassert(psv.epc.expectedOuterPorts.insert(
        std::make_pair(p.getParentPort(), port)).second);
    }
    SCPortBase2Port::iterator iter = psv.expectedOuterPorts.find(&p);
    if (iter != psv.expectedOuterPorts.end()) {
#ifdef SYSTEMOC_ENABLE_DEBUG
      if (outDbg.isVisible(Debug::Low)) {
        outDbg << " => handled expectedOuterPorts " << iter->second.portName << " connected to outer port " << getName(&p) << std::endl;
      }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
      // FIXME: iter->second->otherPorts().insert(port.toPtr());???
      psv.expectedOuterPorts.erase(iter); // handled it!
    }
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpPort::operator ()(smoc_sysc_port &) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  }

};

class DumpFifo: public NamedIdedObjAccess {
public:
  typedef void result_type;
protected:
  GraphSubVisitor &gsv;
public:
  DumpFifo(GraphSubVisitor &gsv)
    : gsv(gsv) {}

  result_type operator ()(smoc_fifo_chan_base &p) {
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpFifo::operator ()(...) [BEGIN] for " << getName(&p) << std::endl;
    }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    
    gsv.ctx.fifoConnections
      << "  <fifo "
              "name=" << DQ(p.name()) << " "
              "size=\"" << p.depthCount() << "\" "
              "initial=\"" << p.usedCount() << "\">\n";
    for (ChanBase::EntryMap::value_type entry : p.getEntries()) {
      SCInterface2Port::iterator iter =
        gsv.expectedChannelConnections.find(entry.first);
      if (iter != gsv.expectedChannelConnections.end()) {
#ifdef SYSTEMOC_ENABLE_DEBUG
        if (outDbg.isVisible(Debug::Low)) {
          outDbg << "DumpFifoBase::connectPort handled expectedChannelConnection " << reinterpret_cast<void *>(iter->first) << std::endl;
        }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
        gsv.ctx.fifoConnections
          << "    <source actor=" << DQ(iter->second.actorName)
          <<             " port=" << DQ(iter->second.portName) << "/>\n";
        gsv.expectedChannelConnections.erase(iter); // handled it!
      }
    }
    for (ChanBase::OutletMap::value_type outlet : p.getOutlets()) {
      SCInterface2Port::iterator iter =
        gsv.expectedChannelConnections.find(outlet.first);
      if (iter != gsv.expectedChannelConnections.end()) {
#ifdef SYSTEMOC_ENABLE_DEBUG
        if (outDbg.isVisible(Debug::Low)) {
          outDbg << "DumpFifoBase::connectPort handled expectedChannelConnection " << reinterpret_cast<void *>(iter->first) << std::endl;
        }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
        gsv.ctx.fifoConnections
          << "    <target actor=" << DQ(iter->second.actorName)
          <<             " port=" << DQ(iter->second.portName) << "/>\n";
        gsv.expectedChannelConnections.erase(iter); // handled it!
      }
    }
    gsv.ctx.fifoConnections
      << "  </fifo>\n";
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpFifo::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  }
};

class DumpActor: public NodeBaseAccess {
public:
  typedef void result_type;
protected:
  GraphSubVisitor &gsv;

public:
  DumpActor(GraphSubVisitor &gsv)
    : gsv(gsv) {}

  result_type operator ()(smoc_actor &a) {
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpActor::operator ()(...) [BEGIN] for " << getName(&a) << std::endl;
    }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    ActorSubVisitor sv(gsv.ctx, gsv);
    recurse(sv, a);

    std::string actorType = demangle(typeid(a).name());
    std::string actorName = a.name();

    {
      std::string key = actorType;
      for (SCPortBase2Port::value_type p : sv.ports) {
        key += ";" + p.second.portName;
        key += p.second.isInput ? "[IN]" : "[OUT]";
      }
      std::string &type = gsv.ctx.actorTypeCache[key];
      if (type.empty()) {
        type = gsv.ctx.actorTypeUniquePool(actorType);
        gsv.ctx.actorTypes
          << "  <actorType name=" << DQ(type) << ">\n";
        for (SCPortBase2Port::value_type p : sv.ports) {
          gsv.ctx.actorTypes
            << "    <port name=" << DQ(p.second.portName)
            << " type=" << (p.second.isInput ? "\"in\"" : "\"out\"") << "/>\n";
        }
        gsv.ctx.actorTypes
          << "  </actorType>\n";
      }
      actorType = type;
    }

    gsv.ctx.actorInstances
      << "  <actorInstance name=" << DQ(actorName) << " type=" << DQ(actorType) << "/>\n";
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpActor::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  }
};

class DumpGraph: public NamedIdedObjAccess {
public:
  typedef void result_type;
protected:
  GraphSubVisitor &gsv;
public:
  DumpGraph(GraphSubVisitor &gsv)
    : gsv(gsv) {}

  result_type operator ()(GraphBase &g) {
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpGraph::operator ()(...) [BEGIN] for " << getName(&g) << std::endl;
    }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
    GraphSubVisitor sv(gsv.ctx, gsv);
    recurse(sv, g);
#ifdef SYSTEMOC_ENABLE_DEBUG
    if (outDbg.isVisible(Debug::Low)) {
      outDbg << "DumpGraph::operator ()(...) [END]" << std::endl;
    }
#endif //defined(SYSTEMOC_ENABLE_DEBUG)
  }
};

void ProcessSubVisitor::operator ()(PortBase &obj) {
  DumpPort(*this)(obj);
}

void ProcessSubVisitor::operator ()(sc_core::sc_port_base &obj) {
  std::cerr << "Ignoring " << obj.name() << std::endl;
}

void GraphSubVisitor::operator ()(GraphBase &obj) {
  DumpGraph(*this)(obj);
}

void GraphSubVisitor::operator ()(smoc_actor &obj) {
  DumpActor(*this)(obj);
}

void GraphSubVisitor::operator ()(sc_core::sc_module &obj) {
  std::cerr << "Ignoring " << obj.name() << std::endl;
}

void GraphSubVisitor::operator ()(smoc_fifo_chan_base &obj) {
  DumpFifo(*this)(obj);
}

void GraphSubVisitor::operator ()(smoc_multireader_fifo_chan_base &obj) {
  std::cerr << "Ignoring " << obj.name() << std::endl;
}

void GraphSubVisitor::operator ()(smoc_multiplex_fifo_chan_base &obj) {
  std::cerr << "Ignoring " << obj.name() << std::endl;
}

void GraphSubVisitor::operator ()(smoc_reset_chan &obj) {
  std::cerr << "Ignoring " << obj.name() << std::endl;
}

GraphSubVisitor::~GraphSubVisitor() {
  // Kick expectedChannelConnections one layer up
  epc.expectedChannelConnections.insert(
    expectedChannelConnections.begin(),
    expectedChannelConnections.end());
  expectedChannelConnections.clear();
}

} // namespace anonymous

void dumpSNG(std::ostream &file, SimulationContextSNGDumping *simCTX, GraphBase &g) {
  SNGDumpCTX              ctx(simCTX);
  ExpectedPortConnections epc;

  file
    << "<?xml version=\"1.0\"?>\n"
       "<networkGraph\n"
       "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
       "  xsi:noNamespaceSchemaLocation=\"sng.xsd\">\n";
  GraphSubVisitor sv(ctx,epc);
  recurse(sv, g);
  // There may be dangling ports => erase them or we get an assertion!
  epc.expectedOuterPorts.clear();
  //epc.expectedChannelConnections.clear();
  file
    << ctx.actorTypes.str()
    << ctx.actorInstances.str()
    << ctx.fifoConnections.str()
    << "</networkGraph>\n";
}

} } // namespace smoc::Detail
