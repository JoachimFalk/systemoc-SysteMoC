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

#include <smoc/sng/transform.hpp>

#include <string>
//#include <cstdio>

//#include <CoSupport/sassert.h>

namespace smoc { namespace SNG {

std::ostream &operator <<(std::ostream &out, Transform  transform) {
  switch (transform) {
    case Transform::FIFOS_NO_MERGING:
      return out << "FIFOS_NO_MERGING";
    case Transform::FIFOS_SAME_CONTENT_MERGING:
      return out << "FIFOS_SAME_CONTENT_MERGING";
    case Transform::FIFOS_SAME_PRODUCER_MERGING:
      return out << "FIFOS_SAME_PRODUCER_MERGING";
    case Transform::CHANS_ARE_DROPPED_NO_MERGING:
      return out << "CHANS_ARE_DROPPED_NO_MERGING";
    case Transform::CHANS_ARE_DROPPED_SAME_CONTENT_MERGING:
      return out << "CHANS_ARE_DROPPED_SAME_CONTENT_MERGING";
  }

  return out;
}

std::istream &operator >>(std::istream &in, Transform &transform) {
  std::string str;

  in >> str;
  if (!in.bad()) {
    if (str == "FIFOS_NO_MERGING") {
      transform = Transform::FIFOS_NO_MERGING;
    } else if (str == "FIFOS_SAME_CONTENT_MERGING") {
      transform = Transform::FIFOS_SAME_CONTENT_MERGING;
    } else if (str == "FIFOS_SAME_PRODUCER_MERGING") {
      transform = Transform::FIFOS_SAME_PRODUCER_MERGING;
    } else if (str == "CHANS_ARE_DROPPED_NO_MERGING") {
      transform = Transform::CHANS_ARE_DROPPED_NO_MERGING;
    } else if (str == "CHANS_ARE_DROPPED_SAME_CONTENT_MERGING") {
      transform = Transform::CHANS_ARE_DROPPED_SAME_CONTENT_MERGING;
    } else {
      in.setstate(std::ios::badbit);
    }
  }
  return in;
}

namespace {

  void transformFifosSameContentMerging(Graph const &g, Graph &gout) {
    Graph::vertex_iterator vIter, vEndIter;

    typedef std::map<Graph::vertex_descriptor, Graph::vertex_descriptor> VertexMap;

    VertexMap vertexMap;

    // Copy over actors, merge channels with same content, and add write edges of actors
    for (boost::tie(vIter, vEndIter) = vertices(g);
         vIter != vEndIter;
         ++vIter) {
      VertexInfo const &vi = g[*vIter];

      if (vi.type != VertexInfo::ACTOR)
        continue;

      Graph::vertex_descriptor vdSrcActor = add_vertex(gout);
      vertexMap.insert(std::make_pair(*vIter, vdSrcActor));
      gout[vdSrcActor] = g[*vIter];

      typedef std::map<std::string, Graph::vertex_descriptor> MergedVertices;

      Graph::out_edge_iterator eIter, eEndIter;
      MergedVertices           mergedVertices;

      for (boost::tie(eIter, eEndIter) = out_edges(*vIter, g);
           eIter != eEndIter;
           ++eIter) {
        Graph::vertex_descriptor vTarget   = target(*eIter, g);
        Graph::vertex_descriptor vdTgtFifo;

        std::pair<MergedVertices::iterator, bool> status =
            mergedVertices.insert(std::make_pair(g[*eIter].name, Graph::null_vertex()));
        if (status.second) {
          vdTgtFifo = status.first->second = add_vertex(gout);
          gout[vdTgtFifo] = g[vTarget];
          Graph::edge_descriptor edWrite = add_edge(vdSrcActor, vdTgtFifo, gout).first;
          gout[edWrite] = g[*eIter];
        } else
          vdTgtFifo = status.first->second;
        vertexMap.insert(std::make_pair(vTarget, vdTgtFifo));
      }
    }

    // Copy over rest of channels and add read edges of actors
    for (boost::tie(vIter, vEndIter) = vertices(g);
         vIter != vEndIter;
         ++vIter) {
      VertexInfo const &vi = g[*vIter];

      if (vi.type != VertexInfo::ACTOR)
        continue;

      Graph::vertex_descriptor vdTgtActor = vertexMap[*vIter];

      Graph::in_edge_iterator eIter, eEndIter;

      for (boost::tie(eIter, eEndIter) = in_edges(*vIter, g);
           eIter != eEndIter;
           ++eIter) {
        Graph::vertex_descriptor vSource = source(*eIter, g);
        Graph::vertex_descriptor vdSrcFifo = vertexMap[vSource];

        Graph::edge_descriptor edRead = add_edge(vdSrcFifo, vdTgtActor, gout).first;
        gout[edRead] = g[*eIter];
      }
    }
  }

} // namespace anonymous

Graph transform(Graph const &g, Transform transform) {
  Graph gout;
  switch (transform) {
    case Transform::FIFOS_SAME_CONTENT_MERGING:
      transformFifosSameContentMerging(g, gout);
      return gout;
    case Transform::FIFOS_SAME_PRODUCER_MERGING:
      return gout; // FIXME: Implement this
    case Transform::CHANS_ARE_DROPPED_NO_MERGING:
      return gout; // FIXME: Implement this
    case Transform::CHANS_ARE_DROPPED_SAME_CONTENT_MERGING:
      return gout; // FIXME: Implement this
  }
  assert(transform == Transform::FIFOS_NO_MERGING);
  return g; // NOP
}

} } // namespace smoc::SNG
