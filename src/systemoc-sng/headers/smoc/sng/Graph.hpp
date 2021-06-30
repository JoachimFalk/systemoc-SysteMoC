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
      out << "[label=\"" << ei.name << "\"]";
  //  out << "[headlabel=\"c:" << g[ed].cons << "\" taillabel=\"p:" << g[ed].prod << "\"]";
  //  out << "[label=\"c:" << g[ed].cons << "\\np:" << g[ed].prod << "\\ns:" << g[ed].capacity << "\\nd:" << g[ed].delay << "\"]";
    }
  };

} } // namespace smoc::SNG

#endif /* _INCLUDED_SMOC_SNG_GRAPH_HPP */
