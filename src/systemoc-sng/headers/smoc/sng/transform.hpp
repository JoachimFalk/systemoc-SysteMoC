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

#ifndef _INCLUDED_SMOC_SNG_TRANSFORM_HPP
#define _INCLUDED_SMOC_SNG_TRANSFORM_HPP

#include "Graph.hpp"

#include <iostream>

namespace smoc { namespace SNG {

  enum class Transform {
      FIFOS_NO_MERGING /* This is a nop */
    , FIFOS_SAME_CONTENT_MERGING
    , FIFOS_SAME_PRODUCER_MERGING
    , CHANS_ARE_DROPPED_NO_MERGING
    , CHANS_ARE_DROPPED_SAME_CONTENT_MERGING
  };

  std::ostream &operator <<(std::ostream &out, Transform  transform);
  std::istream &operator >>(std::istream &in,  Transform &transform);

  Graph transform(Graph const &g, Transform transform);

} } // namespace smoc::SNG

#endif /* _INCLUDED_SMOC_SNG_TRANSFORM_HPP */
