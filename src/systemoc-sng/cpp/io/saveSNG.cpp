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
#include <CoSupport/String/XMLQuotedString.hpp>
#include <CoSupport/String/UniquePool.hpp>

#include <CoSupport/sassert.h>

namespace smoc { namespace SNG {

typedef CoSupport::String::XMLQuotedString XQ;

namespace {
  struct SNGDumpCTX {
    std::map<std::string, std::string>  actorTypeCache;
    CoSupport::String::UniquePool       actorTypeUniquePool;

    std::stringstream actorTypes;
    std::stringstream actorInstances;
    std::stringstream fifoConnections;

  };
}

void  saveSNG(Graph const &g, std::ostream &out) {

  SNGDumpCTX ctx;

  out
    << "<?xml version=\"1.0\"?>\n"
       "<networkGraph\n"
       "  xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\"\n"
       "  xsi:noNamespaceSchemaLocation=\"sng.xsd\">\n";

  for (std::pair<
           Graph::vertex_iterator
         , Graph::vertex_iterator> vip = vertices(g);
       vip.first != vip.second;
       ++vip.first)
    switch (g[*vip.first].type) {
      case VertexInfo::ACTOR: {
        std::string key = "A";
        for (std::pair<
                Graph::in_edge_iterator
              , Graph::in_edge_iterator> eip = in_edges(*vip.first, g);
             eip.first != eip.second;
             ++eip.first)
        {
          key += g[*eip.first].name;
          key += "[IN]";
        }
        for (std::pair<
                Graph::out_edge_iterator
              , Graph::out_edge_iterator> eip = out_edges(*vip.first, g);
             eip.first != eip.second;
             ++eip.first)
        {
          key += g[*eip.first].name;
          key += "[OUT]";
        }
        std::string &actorType = ctx.actorTypeCache[key];
        if (actorType.empty()) {
          actorType = ctx.actorTypeUniquePool("A");
          ctx.actorTypes << "  <actorType name=" << XQ(actorType) << ">\n";
          for (std::pair<
                  Graph::in_edge_iterator
                , Graph::in_edge_iterator> eip = in_edges(*vip.first, g);
               eip.first != eip.second;
               ++eip.first)
            ctx.actorTypes << "    <port name=" << XQ(g[*eip.first].name) << " type=\"in\"/>\n";
          for (std::pair<
                  Graph::out_edge_iterator
                , Graph::out_edge_iterator> eip = out_edges(*vip.first, g);
               eip.first != eip.second;
               ++eip.first)
            ctx.actorTypes << "    <port name=" << XQ(g[*eip.first].name) << " type=\"out\"/>\n";
          ctx.actorTypes << "  </actorType>\n";
        }
        std::string const &actorName = g[*vip.first].name;
        ctx.actorInstances << "  <actorInstance name=" << XQ(actorName) << " type=" << XQ(actorType) << "/>\n";
        break;
      }
      case VertexInfo::FIFO: {
        for (std::pair<
                Graph::out_edge_iterator
              , Graph::out_edge_iterator> eip = out_edges(*vip.first, g);
             eip.first != eip.second;
             ++eip.first) {
          ctx.fifoConnections
            << "  <fifo "
                    "name=" << XQ(g[*vip.first].name) << " "
                    "size=\"" << g[*vip.first].fifo.capacity << "\" "
                    "initial=\"" << g[*vip.first].fifo.delay << "\">\n"
               "    <opendseattr name=\"smoc-token-size\" type=\"INT\" value=\"" << g[*vip.first].fifo.tokenSize << "\"/>\n";
          for (std::pair<
                 Graph::in_edge_iterator
               , Graph::in_edge_iterator> eip = in_edges(*vip.first, g);
              eip.first != eip.second;
              ++eip.first) {
            ctx.fifoConnections
              << "    <source actor=" << XQ(g[boost::source(*eip.first, g)].name)
              <<             " port=" << XQ(g[*eip.first].name) << "/>\n";
          }
          ctx.fifoConnections
            << "    <target actor=" << XQ(g[boost::target(*eip.first, g)].name)
            <<             " port=" << XQ(g[*eip.first].name) << "/>\n";
          ctx.fifoConnections
            << "  </fifo>\n";
        }
        break;
      }
      case VertexInfo::REGISTER: {
        ctx.fifoConnections
            << "  <register "
                    "name=" << XQ(g[*vip.first].name) << ">\n"
               "    <opendseattr name=\"smoc-token-size\" type=\"INT\" value=\"" << g[*vip.first].reg.tokenSize << "\"/>\n";
        for (std::pair<
               Graph::in_edge_iterator
             , Graph::in_edge_iterator> eip = in_edges(*vip.first, g);
            eip.first != eip.second;
            ++eip.first) {
          ctx.fifoConnections
            << "    <source actor=" << XQ(g[boost::source(*eip.first, g)].name)
            <<             " port=" << XQ(g[*eip.first].name) << "/>\n";
        }

        for (std::pair<
                Graph::out_edge_iterator
              , Graph::out_edge_iterator> eip = out_edges(*vip.first, g);
             eip.first != eip.second;
             ++eip.first) {
          ctx.fifoConnections
            << "    <target actor=" << XQ(g[boost::target(*eip.first, g)].name)
            <<             " port=" << XQ(g[*eip.first].name) << "/>\n";
        }
        assert(in_edges(*vip.first, g).first == in_edges(*vip.first, g).second || out_edges(*vip.first, g).first == out_edges(*vip.first, g).second);
        ctx.fifoConnections
          << "  </register>\n";
        break;
      }
    }
  out
    << ctx.actorTypes.str()
    << ctx.actorInstances.str()
    << ctx.fifoConnections.str()
    << "</networkGraph>\n";
}

} } // namespace smoc::SNG
