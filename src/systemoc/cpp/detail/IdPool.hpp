// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2017 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_IDPOOL_HPP
#define _INCLUDED_SMOC_DETAIL_IDPOOL_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_NEED_IDS

#include <map>
#include <limits>

#include <CoSupport/compatibility-glue/nullptr.h>

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

#endif // _INCLUDED_SMOC_DETAIL_IDPOOL_HPP
