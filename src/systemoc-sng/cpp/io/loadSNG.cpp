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

#include <smoc/sng/io.hpp>

#include <iostream>
#include <sstream>
#include <string>

//#include <cstdio>

#include <CoSupport/sassert.h>

#include <CoSupport/XML/Xerces/Handler.hpp>

#include "sng-xsd.c"

namespace XN = ::CoSupport::XML::Xerces::XN;

using ::CoSupport::XML::Xerces::XStr;
using ::CoSupport::XML::Xerces::NStr;

namespace smoc { namespace SNG {

namespace {

  typedef Graph::vertex_descriptor VD;
  typedef Graph::edge_descriptor   ED;

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
      , Graph &g
      , ActorTypes const &actorTypes)
      : name(domActorInstance->getAttribute(XMLCH("name")))
      , type(findType(domActorInstance, actorTypes))
      , vd(add_vertex(g))
    {
      VertexInfo &vi = g[vd];
      vi.name = getName();
      vi.type = VertexInfo::ACTOR;

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
      , Graph &g
      , ActorInstances &actorInstances)
      : name(domChanInstance->getAttribute(XMLCH("name")))
      , vd(add_vertex(g))
    {
      VertexInfo &vi = g[vd];
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
          EdgeInfo &ei = g[eStatus.first];
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
          EdgeInfo &ei = g[eStatus.first];
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
      , Graph &g
      , ActorInstances &actorInstances)
      : ChanInstance(domFifoInstance, g, actorInstances)
    {
      VertexInfo &vi = g[vd];
      vi.type = VertexInfo::FIFO;
      vi.fifo.capacity = std::stoul(NStr(domFifoInstance->getAttribute(XMLCH("size"))));
      vi.fifo.delay    = std::stoul(NStr(domFifoInstance->getAttribute(XMLCH("initial"))));
    }
  };

  class RegisterInstance: public ChanInstance {
  public:
    RegisterInstance(
        XN::DOMElement *domFifoInstance
      , Graph &g
      , ActorInstances &actorInstances)
      : ChanInstance(domFifoInstance, g, actorInstances)
    {
      VertexInfo &vi = g[vd];
      vi.type = VertexInfo::REGISTER;
    }
  };

}

Graph loadSNG(std::istream &in) {

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

  Graph sngGraph;

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

} } // namespace smoc::SNG
