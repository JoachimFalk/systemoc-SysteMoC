// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2021 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_SNG_GRAPH_HPP
#define _INCLUDED_SMOC_SNG_GRAPH_HPP

#include <boost/graph/adjacency_list.hpp>
#include <boost/graph/properties.hpp>

#include <string>
#include <iostream>

namespace smoc { namespace SNG {

  struct ActorInfo {
    size_t       repCount;  ///< repetition vector count
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

    VertexInfo()
      : type(FIFO) {
      fifo.tokenSize = -1;
      fifo.capacity  = -1;
      fifo.delay     = -1;
    }
  };

  struct EdgeInfo {
    std::string name;
    size_t      tokenSize;
    size_t      tokens;

    EdgeInfo()
      : tokenSize(-1), tokens(-1) {}
  };

  typedef ::boost::adjacency_list<
    // edge container type
    ::boost::vecS,
    // vertex container type
    ::boost::vecS,
    // direction type
    ::boost::bidirectionalS,
    // vertex properties
    VertexInfo,
    // edge properties
    EdgeInfo,
    // graph properties
    ::boost::no_property // don't need graph properties
  > Graph;

  typedef Graph::vertex_descriptor VertexDescriptor;
  typedef Graph::vertex_iterator   VertexIterator;
  typedef std::pair<
      VertexIterator
    , VertexIterator>              VertexIteratorPair;

  typedef Graph::edge_descriptor   EdgeDescriptor;
  typedef Graph::edge_iterator     EdgeIterator;
  typedef std::pair<
      EdgeIterator
    , EdgeIterator>                EdgeIteratorPair;

  typedef boost::property_map<
    Graph, boost::vertex_index_t>::type          VertexIndexMap;
  typedef boost::property_map<
    Graph, std::string VertexInfo::*>::type      VertexNameMap;
  typedef boost::property_map<
    Graph, VertexInfo::Type VertexInfo::*>::type VertexTypeMap;

  class ActorRepCountMap: public boost::put_get_helper<size_t &, ActorRepCountMap> {
  public:
    typedef VertexDescriptor                    key_type;
    typedef size_t                              value_type;
    typedef value_type                         &reference;
    typedef boost::read_write_property_map_tag  category;

    ActorRepCountMap(Graph &g)
      : g(g) {}

    reference operator[](const key_type &key) const
      { return g[key].actor.repCount; }
  private:
    Graph &g;
  };

  class ChannelTSizeMap: public boost::put_get_helper<size_t &, ChannelTSizeMap> {
  public:
    typedef VertexDescriptor                    key_type;
    typedef size_t                              value_type;
    typedef value_type                         &reference;
    typedef boost::read_write_property_map_tag  category;

    ChannelTSizeMap(Graph &g)
      : g(g) {}

    reference operator[](const key_type &key) const
      { return g[key].fifo.tokenSize; }
  private:
    Graph &g;
  };

  class FIFODelayMap: public boost::put_get_helper<size_t &, FIFODelayMap> {
  public:
    typedef VertexDescriptor                    key_type;
    typedef size_t                              value_type;
    typedef value_type                         &reference;
    typedef boost::read_write_property_map_tag  category;

    FIFODelayMap(Graph &g)
      : g(g) {}

    reference operator[](const key_type &key) const
      { return g[key].fifo.delay; }
  private:
    Graph &g;
  };

  class FIFOCapacityMap: public boost::put_get_helper<size_t &, FIFOCapacityMap> {
  public:
    typedef VertexDescriptor                    key_type;
    typedef size_t                              value_type;
    typedef value_type                         &reference;
    typedef boost::read_write_property_map_tag  category;

    FIFOCapacityMap(Graph &g)
      : g(g) {}

    reference operator[](const key_type &key) const
      { return g[key].fifo.capacity; }
  private:
    Graph &g;
  };

  typedef boost::property_map<
    Graph, std::string EdgeInfo::*>::type EdgeNameMap;
  typedef boost::property_map<
    Graph, size_t EdgeInfo::*>::type      EdgeTokenSizeMap;
  typedef boost::property_map<
    Graph, size_t EdgeInfo::*>::type      EdgeTokensMap;

  /// Basic vertex property writer for SNG graphs.
  /// Property writers are used in dot file generation, which are mainly used for debugging purpose.
  /// \sa EdgePropertyWriter
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
          if (vi.actor.repCount != static_cast<size_t>(-1))
            out << "[label=\"" << vi.name << "(x" << vi.actor.repCount << ")\" style=\"filled\" fillcolor=\"#ff684c\" shape=\"box\"]";
          else
            out << "[label=\"" << vi.name << "\" style=\"filled\" fillcolor=\"#ff684c\"]";
          break;
        case VertexInfo::FIFO:
          out << "[label=\"" << vi.name;
          if (vi.fifo.delay != 0)
            out << ", d:" << vi.fifo.delay;
          if (vi.fifo.capacity != static_cast<size_t>(-1)) {
            out << ", c:" << vi.fifo.capacity;
            if (vi.fifo.tokenSize != static_cast<size_t>(-1))
              out << ", s:" << vi.fifo.capacity * vi.fifo.tokenSize;
          }
          out << "\" style=\"filled\" fillcolor=\"#f2e898\" shape=\"box\"]";
          break;
        case VertexInfo::REGISTER:
          out << "[label=\"" << vi.name;
          if (vi.reg.tokenSize != static_cast<size_t>(-1))
            out << ", s:" << vi.reg.tokenSize;
          out << "\" style=\"filled\" fillcolor=\"#f2e898\" shape=\"box\"]";
          break;
      }
  //  out << "[label=\"" << get(&VertexInfo::name, g, vd) << "(x" << g[vd].repCount << ")\" shape=\"box\"]";
    }
  };

  /// Basic edge property writer for SNG graphs.
  /// Property writers are used in dot file generation, which are mainly used for debugging purpose.
  /// \sa VertexPropertyWriter
  class EdgePropertyWriter {
    typedef EdgePropertyWriter this_type;
  protected:
    Graph &g;
  public:
    EdgePropertyWriter(Graph &g): g(g) {}

    void operator()(std::ostream &out, Graph::edge_descriptor ed) {
      EdgeInfo const &ei = g[ed];

      out << "[label=\"" << ei.name;
      bool commaReq = !ei.name.empty();
      if (ei.tokens != static_cast<size_t>(-1)) {
        out << (commaReq ? ", t:" : "t:") << ei.tokens;
        if (ei.tokenSize != static_cast<size_t>(-1))
          out << ", s:" << ei.tokenSize*ei.tokens;
        commaReq = true;
      }
      out << "\"]";
  //  out << "[headlabel=\"c:" << g[ed].cons << "\" taillabel=\"p:" << g[ed].prod << "\"]";
  //  out << "[label=\"c:" << g[ed].cons << "\\np:" << g[ed].prod << "\\ns:" << g[ed].capacity << "\\nd:" << g[ed].delay << "\"]";
    }
  };

} } // namespace smoc::SNG

#endif /* _INCLUDED_SMOC_SNG_GRAPH_HPP */
