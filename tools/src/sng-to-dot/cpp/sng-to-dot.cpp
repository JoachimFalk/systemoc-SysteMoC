// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *  2021 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include <iostream>
#include <sstream>
#include <string>

#include <cstdio>

#include <CoSupport/Streams/DebugOStream.hpp>
#include <CoSupport/Streams/AlternateStream.hpp>
#include <CoSupport/Streams/NullStreambuf.hpp>

#include <CoSupport/sassert.h>

#include <CoSupport/XML/Xerces/Handler.hpp>

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>
#include <boost/program_options/parsers.hpp>
#include <boost/program_options/positional_options.hpp>

#include <boost/filesystem/path.hpp>
#include <boost/filesystem/convenience.hpp>

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/filtered_graph.hpp>
#include <boost/graph/connected_components.hpp>
//#include <boost/graph/strong_components.hpp>
#include <boost/graph/graphviz.hpp>
//#include <boost/graph/topological_sort.hpp>
#include <CoSupport/boost/graph/undirect_graph.hpp>
#include <CoSupport/boost/graph/graph_traits.hpp>
#include <CoSupport/boost/graph/properties.hpp>

#include <boost/property_map/property_map.hpp>
#include <CoSupport/boost/property_map/compose_property_map.hpp>
//#include <boost/property_map/compose_property_map.hpp>
//#include <boost/graph/property_maps/container_property_map.hpp>

//#include <nganalysis/SDF.hpp>
//#include <nganalysis/CSDF.hpp>

#include "sng-xsd.c"

std::string prgname = "???";

namespace po = ::boost::program_options;
namespace fs = ::boost::filesystem;
namespace XN = ::CoSupport::XML::Xerces::XN;

using ::CoSupport::Streams::Debug;
//using ::CoSupport::Streams::ScopedIndent;
using ::CoSupport::XML::Xerces::XStr;
using ::CoSupport::XML::Xerces::NStr;

typedef
#ifdef NDEBUG
  CoSupport::Streams::NullStreambuf::Stream<
#endif //NDEBUG
    CoSupport::Streams::DebugStreambuf::Stream<
      CoSupport::Streams::IndentStreambuf::Stream<
        CoSupport::Streams::HeaderFooterStreambuf::Stream<
    > > >
#ifdef NDEBUG
  >
#endif //NDEBUG
  DebugOStream;

/// Debug output stream
DebugOStream outDbg(std::cerr);

namespace smoc { namespace sng {

  struct ActorInfo {

  };

  struct FIFOInfo {
    size_t       tokenSize; ///< size of a token in bytes
    size_t       capacity;  ///< size of the FIFO buffer in tokens
    size_t       delay;     ///< number of initial tokens
  };

  struct RegisterInfo {
    size_t       tokenSize; ///< size of a token in bytes
  };

  struct VertexInfo {
    enum Type { ACTOR, FIFO, REGISTER };

    std::string    name;
    Type           type;
    union {
      ActorInfo    actor;
      FIFOInfo     fifo;
      RegisterInfo reg;
    };
  };

  struct EdgeInfo {
    std::string name;
    int         tokenSize;
    int         tokens;
  };

  typedef boost::adjacency_list<
    // edge container type
    boost::vecS,
    // vertex container type
    boost::vecS,
    // direction type
    boost::bidirectionalS,
    // vertex properties
    VertexInfo,
    // edge properties
    EdgeInfo,
    // graph properties
    boost::no_property // don't need graph properties
  > Graph;

  /// Basic vertex property writer for SDF.
  /// Property writers are used in dot file generation, which are mainly used for debugging purpose.
  /// \sa SDF::EdgePropertyWriter
  class VertexPropertyWriter {
    typedef VertexPropertyWriter this_type;
  protected:
    Graph &g;
  public:
    VertexPropertyWriter(Graph &g): g(g) {}

    void operator()(std::ostream &out, Graph::vertex_descriptor vd) {
      VertexInfo const &vi = g[vd];

      switch (vi.type) {
        case VertexInfo::ACTOR:
          out << "[label=\"" << vi.name << "\" style=\"filled\" fillcolor=\"#ff684c\"]";
          break;
        case VertexInfo::FIFO:
          out << "[label=\"" << vi.name << "\" style=\"filled\" fillcolor=\"#f2e898\" shape=\"box\"]";
          break;
        case VertexInfo::REGISTER:
          out << "[label=\"" << vi.name << "\" style=\"filled\" fillcolor=\"#f2e898\" shape=\"box\"]";
          break;
      }
  //  out << "[label=\"" << get(&VertexInfo::name, g, vd) << "(x" << g[vd].repCount << ")\" shape=\"box\"]";
    }
  };

  /// Basic edge property writer for SDF.
  /// Property writers are used in dot file generation, which are mainly used for debugging purpose.
  /// \sa SDF::VertexPropertyWriter
  class EdgePropertyWriter {
    typedef EdgePropertyWriter this_type;
  protected:
    Graph &g;
  public:
    EdgePropertyWriter(Graph &g): g(g) {}

    void operator()(std::ostream &out, Graph::edge_descriptor ed) {
      EdgeInfo const &ei = g[ed];
      out << "[label=\"" << ei.name << "\"]";
  //  out << "[headlabel=\"c:" << g[ed].cons << "\" taillabel=\"p:" << g[ed].prod << "\"]";
  //  out << "[label=\"c:" << g[ed].cons << "\\np:" << g[ed].prod << "\\ns:" << g[ed].capacity << "\\nd:" << g[ed].delay << "\"]";
    }
  };

} } // namespace smoc::SNG

//  typedef boost::property_map<
//    graph, boost::vertex_index_t>::type           PropVertexIndexMap;
//  typedef boost::property_map<
//    graph, std::string VertexInfo::*>::type       PropVertexNameMap;
//  typedef boost::property_map<
//    graph, size_t VertexInfo::*>::type            PropVertexRepCountMap;
//
//  typedef boost::property_map<
//    graph, std::string EdgeInfo::*>::type         PropEdgeNameMap;
//  typedef boost::property_map<
//    graph, size_t EdgeInfo::*>::type              PropEdgeSizeTMap;


typedef smoc::sng::Graph::vertex_descriptor VD;
typedef smoc::sng::Graph::edge_descriptor   ED;

class ActorPort {
public:
  enum Direction { IN, OUT };

  ActorPort(XN::DOMElement *domActorPort)
    : name(domActorPort->getAttribute(XMLCH("name")))
  {
    XMLCh const *d = domActorPort->getAttribute(XMLCH("type"));
    assert(
        XN::XMLString::compareString(XMLCH("in"), d)  == 0
     || XN::XMLString::compareString(XMLCH("out"), d) == 0);
    direction = XN::XMLString::compareString(XMLCH("in"), d) == 0
        ? IN : OUT;
  }

  std::string const &getName() const
    { return name; }

  Direction          getDirection() const
    { return direction; }

protected:
  NStr      name;
  Direction direction;
};

typedef std::map<std::string, ActorPort> ActorPorts;

class ActorType {

public:
  ActorType(XN::DOMElement *domActorType)
    : type(domActorType->getAttribute(XMLCH("name")))
  {
    const XN::DOMNodeList *domActorPorts = domActorType->getElementsByTagName(XMLCH("port"));
    for (size_t i = 0; i < domActorPorts->getLength(); ++i) {
      ActorPort actorPort(static_cast<XN::DOMElement *>(domActorPorts->item(i)));
      sassert(ports.insert(std::make_pair(actorPort.getName(), actorPort)).second);
    }
  }

  std::string const &getType() const
    { return type; }

  ActorPorts  const &getPorts() const
    { return ports; }

protected:
  NStr       type;
  ActorPorts ports;
};

typedef std::map<std::string, ActorType> ActorTypes;

class ActorInstance {

public:
  ActorInstance(
      XN::DOMElement *domActorInstance
    , smoc::sng::Graph &g
    , ActorTypes const &actorTypes)
    : name(domActorInstance->getAttribute(XMLCH("name")))
    , type(findType(domActorInstance, actorTypes))
    , vd(add_vertex(g))
  {
    smoc::sng::VertexInfo &vi = g[vd];
    vi.name = getName();
    vi.type = smoc::sng::VertexInfo::ACTOR;

    for (ActorPorts::value_type const &e : type.getPorts())
      sassert(danglingPorts.insert(e.first).second);
  }

  std::string const &getName() const
    { return name; }

  ActorType   const &getType() const
    { return type; }

  VD                 getVD() const
    { return vd; }

  ActorPort   const &connectPort(std::string const &name) {
    danglingPorts.erase(name);
    ActorPorts::const_iterator iter = type.getPorts().find(name);
    assert(iter != type.getPorts().end());
    return iter->second;
  }
protected:
  static ActorType const &findType(
      XN::DOMElement *domActorInstance
    , ActorTypes const &actorTypes)
  {
    NStr type(domActorInstance->getAttribute(XMLCH("type")));

    ActorTypes::const_iterator iter = actorTypes.find(type);
    assert(iter != actorTypes.end());
    return iter->second;
  }

  typedef std::set<std::string> DanglingPorts;

  NStr             name;
  ActorType const &type;
  VD               vd;
  DanglingPorts    danglingPorts;
};

typedef std::map<std::string, ActorInstance> ActorInstances;

class ChanInstance {
public:
  ChanInstance(
      XN::DOMElement *domChanInstance
    , smoc::sng::Graph &g
    , ActorInstances &actorInstances)
    : name(domChanInstance->getAttribute(XMLCH("name")))
    , vd(add_vertex(g))
  {
    smoc::sng::VertexInfo &vi = g[vd];
    vi.name = getName();
    {
      const XN::DOMNodeList *domSources = domChanInstance->getElementsByTagName(XMLCH("source"));
      for (size_t i = 0; i < domSources->getLength(); ++i) {
        XN::DOMElement *domSource = static_cast<XN::DOMElement *>(domSources->item(i));
        NStr actorName(domSource->getAttribute(XMLCH("actor")));
        ActorInstances::iterator iter = actorInstances.find(actorName);
        assert(iter != actorInstances.end());
        NStr portName(domSource->getAttribute(XMLCH("port")));
        ActorPort const &port = iter->second.connectPort(portName);
        assert(port.getDirection() == ActorPort::OUT);

        std::pair<ED, bool> eStatus = boost::add_edge(iter->second.getVD(), vd, g);
        assert(eStatus.second && "WTF?! Failed to insert edge into boost graph g!");
        smoc::sng::EdgeInfo &ei = g[eStatus.first];
        ei.name = portName;
      }
    }
    {
      const XN::DOMNodeList *domTargets = domChanInstance->getElementsByTagName(XMLCH("target"));
      for (size_t i = 0; i < domTargets->getLength(); ++i) {
        XN::DOMElement *domTarget = static_cast<XN::DOMElement *>(domTargets->item(i));
        NStr actorName(domTarget->getAttribute(XMLCH("actor")));
        ActorInstances::iterator iter = actorInstances.find(actorName);
        assert(iter != actorInstances.end());
        NStr portName(domTarget->getAttribute(XMLCH("port")));
        ActorPort const &port = iter->second.connectPort(portName);
        assert(port.getDirection() == ActorPort::IN);

        std::pair<ED, bool> eStatus = boost::add_edge(vd, iter->second.getVD(), g);
        assert(eStatus.second && "WTF?! Failed to insert edge into boost graph g!");
        smoc::sng::EdgeInfo &ei = g[eStatus.first];
        ei.name = portName;
      }
    }
  }

  std::string const &getName()
    { return name; }

protected:
  static ActorInstance const &findActor(
      XN::DOMElement *domActorInstance
    , ActorInstances const &actorInstances)
  {
    NStr type(domActorInstance->getAttribute(XMLCH("type")));

    ActorInstances::const_iterator iter = actorInstances.find(type);
    assert(iter != actorInstances.end());
    return iter->second;
  }

  NStr   name;
  VD     vd;
};

class FifoInstance: public ChanInstance {
public:
  FifoInstance(
      XN::DOMElement *domFifoInstance
    , smoc::sng::Graph &g
    , ActorInstances &actorInstances)
    : ChanInstance(domFifoInstance, g, actorInstances)
  {
    smoc::sng::VertexInfo &vi = g[vd];
    vi.type = smoc::sng::VertexInfo::FIFO;
    vi.fifo.capacity = std::stoul(NStr(domFifoInstance->getAttribute(XMLCH("size"))));
    vi.fifo.delay    = std::stoul(NStr(domFifoInstance->getAttribute(XMLCH("initial"))));
  }
};

class RegisterInstance: public ChanInstance {
public:
  RegisterInstance(
      XN::DOMElement *domFifoInstance
    , smoc::sng::Graph &g
    , ActorInstances &actorInstances)
    : ChanInstance(domFifoInstance, g, actorInstances)
  {
    smoc::sng::VertexInfo &vi = g[vd];
    vi.type = smoc::sng::VertexInfo::REGISTER;
  }
};

smoc::sng::Graph loadSNG(std::istream &in) {

  CoSupport::XML::Xerces::Handler sng;

  sng.setTopElementName(XMLCH("networkGraph"));
  sng.setXSDUrl(XMLCH("sng.xsd"));
  sng.setXSD(sngXSD, sizeof(sngXSD));
  sng.load(in);

  const XN::DOMDocument *domDoc = sng.getDocument();
  const XN::DOMElement  *domTop = domDoc->getDocumentElement();
  assert(XStr(domTop->getTagName()) == XMLCH("networkGraph"));

  ActorTypes actorTypes;

  {
    const XN::DOMNodeList *domActorTypes = domTop->getElementsByTagName(XMLCH("actorType"));
    for (size_t i = 0; i < domActorTypes->getLength(); ++i) {
      ActorType actorType(static_cast<XN::DOMElement *>(domActorTypes->item(i)));
      sassert(actorTypes.insert(std::make_pair(actorType.getType(), actorType)).second);
    }
  }

  smoc::sng::Graph sngGraph;

  ActorInstances actorInstances;

  {
    const XN::DOMNodeList *domActorInstances = domTop->getElementsByTagName(XMLCH("actorInstance"));
    for (size_t i = 0; i < domActorInstances->getLength(); ++i) {
      ActorInstance actorInstance(
          static_cast<XN::DOMElement *>(domActorInstances->item(i))
        , sngGraph
        , actorTypes);
      sassert(actorInstances.insert(std::make_pair(actorInstance.getName(), actorInstance)).second);
    }
  }

  {
    const XN::DOMNodeList *domFifoInstances = domTop->getElementsByTagName(XMLCH("fifo"));
    for (size_t i = 0; i < domFifoInstances->getLength(); ++i) {
      FifoInstance fifoInstance(
          static_cast<XN::DOMElement *>(domFifoInstances->item(i))
        , sngGraph
        , actorInstances);
    }
  }

  {
    const XN::DOMNodeList *domRegisterInstances = domTop->getElementsByTagName(XMLCH("register"));
    for (size_t i = 0; i < domRegisterInstances->getLength(); ++i) {
      RegisterInstance registerInstance(
          static_cast<XN::DOMElement *>(domRegisterInstances->item(i))
        , sngGraph
        , actorInstances);
    }
  }

  return sngGraph;
}


std::pair<std::string, std::string> poNoPrefixHandler(const std::string &s) {
  if (s.find("--no-") == 0) {
    return std::make_pair(s.substr(5), std::string("false"));
  } else {
    return std::make_pair(std::string(), std::string());
  }
}

/*
 * program [options] <network_graph>
 */
int main(int argc, char** argv)
{
  if (argc >= 1 && argv[0] != nullptr)
    prgname = basename(fs::path(argv[0]));
  
  std::stringstream sstr;
  
  sstr << "Usage: " << prgname << " [options] <input> <output>" << std::endl;
  sstr << "Available options";
  
  po::options_description publicOptions(sstr.str());
  
#ifndef NDEBUG
  sstr.str(""); // reset string stream
  sstr << "set debug level; level 0 is off; level " << Debug::None.level << " is most verbose";
#endif //SGXUTILS_DEBUG_OUTPUT
  
  publicOptions.add_options()
    ("help", "produce help message")
#ifndef NDEBUG
    ("debug", po::value<size_t>()->default_value(0), sstr.str().c_str())
#endif //SGXUTILS_DEBUG_OUTPUT
    ("sng" , po::value<std::string>()->default_value("-"), "input sng file")
    ("dot" , po::value<std::string>()->default_value("-"), "output dot file")
    ;

  po::options_description privateOptions;
  privateOptions.add_options()
#ifndef SGXUTILS_DEBUG_OUTPUT
    ("debug", po::value<size_t>()->default_value(0), "turn on debug mode")
#endif //SGXUTILS_DEBUG_OUTPUT
    ;
  
  po::positional_options_description pod;
  pod.add("sng", 1);
  pod.add("dot", 1);
  
  // All options
  po::options_description od;
  od.add(publicOptions);
  od.add(privateOptions);
  
  po::variables_map vm;
  
  try {
    store(po::command_line_parser(argc, argv)
      .options(od)
      .positional(pod)
      .extra_parser(poNoPrefixHandler)
      .run(), vm);
    notify(vm);
  
    if (vm.count("help")) {
      std::cout << publicOptions << std::endl;
      return 0;
    }
    if (!vm.count("sng")) {
      throw std::runtime_error("no sng file for input specified");
    }
    if (!vm.count("dot")) {
      throw std::runtime_error("no dot file for output specified");
    }
    
#ifndef NDEBUG
    int   debugLevel = Debug::None.level - vm["debug"].as<size_t>();
    outDbg.setLevel(debugLevel < 0 ? 0 : debugLevel);
    outDbg << Debug::High;
#else  //NDEBUG
    if (vm["debug"].as<size_t>() != 0)
      std::cerr << prgname << ": Warning --debug support not compiled in!" << std::endl;
#endif //NDEBUG
    
    CoSupport::Streams::AIStream in(std::cin, vm["sng"].as<std::string>(), "-");
    CoSupport::Streams::AOStream out(std::cout, vm["dot"].as<std::string>(), "-");
    
    CoSupport::XML::Xerces::Handler sng();

    smoc::sng::Graph sngGraph = loadSNG(in);

    smoc::sng::VertexPropertyWriter vpw(sngGraph);
    smoc::sng::EdgePropertyWriter   epw(sngGraph);
    boost::write_graphviz(out, sngGraph, vpw, epw);
  } catch (std::exception &e) {
    std::cerr << prgname << ": " << e.what() << std::endl;
    return 1;
  }
  
  return 0;
}
