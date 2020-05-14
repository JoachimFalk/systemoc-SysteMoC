// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_IDPOOL_HPP
#define _INCLUDED_SMOC_DETAIL_IDPOOL_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_NEED_IDS

#include <map>
#include <limits>

#include <CoSupport/Allocators/IdPool.hpp>

#include <smoc/detail/NamedIdedObj.hpp>

//#include "IdTAllocRequest.hpp"

namespace smoc { namespace Detail {

class IdPool;

struct IdPoolTypes {

  /// @brief Value type of IdMap
  struct IdMapEntry {
    /// @brief NodeBase defining the Id
    IdedObj *node;

    /// @brief Default constructor
    IdMapEntry(IdedObj *node = nullptr)
      : node(node) {}
  };

  /// @brief Maps a specific Id to its node
  typedef std::map<NgId, IdMapEntry> IdMap;

  // This stuff should not collide with the definitions in
  // LibSGX/implementation/IdPool.hpp
  struct IdRangeAnon {
    static const size_t bits = std::numeric_limits<NgId>::digits - 2;
    static const NgId min = (0ULL << bits);
    static const NgId max = (1ULL << bits) - 1;
  };

  // This stuff should not collide with the definitions in
  // LibSGX/implementation/IdPool.hpp
  struct IdRangeNamed {
    static const size_t bits = std::numeric_limits<NgId>::digits - 2;
    static const NgId min = (2ULL << bits);
    static const NgId max = (3ULL << bits) - 1;
  };

  typedef CoSupport::Allocators::IdAllocator<IdPool, IdMap, IdRangeAnon>  IdAllocAnon;
  typedef CoSupport::Allocators::IdAllocator<IdPool, IdMap, IdRangeNamed> IdAllocNamed;

};

class IdPool
: public IdPoolTypes::IdAllocAnon,
  public IdPoolTypes::IdAllocNamed,
  public IdPoolTypes,
  public NamedIdedObjAccess {
public:
  typedef IdPoolTypes::IdMap IdMap;

  friend class CoSupport::Allocators::IdAllocator<IdPool, IdMap, IdRangeAnon>;
  friend class CoSupport::Allocators::IdAllocator<IdPool, IdMap, IdRangeNamed>;
protected:
  /// @brief Id -> object lookup map
  IdMap idMap;
public:
  IdedObj *getNodeById(const NgId id) const;

  // set "id"-attribute to some new id
  void addIdedObj(IdedObj *n);

  // set "id"-attribute to id
  void addIdedObj(NamedIdedObj *n);

  // Register IdedObj *n with given id and set "id"-attribute of n.
  void addIdedObj(const NgId id, IdedObj *n);
};

} } // namespace smoc::Detail

#endif // SYSTEMOC_NEED_IDS

#endif /* _INCLUDED_SMOC_DETAIL_IDPOOL_HPP */
