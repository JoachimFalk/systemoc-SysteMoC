// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifdef SYSTEMOC_NEED_IDS

#include "IdPool.hpp"

namespace smoc { namespace Detail {

IdedObj *IdPool::getNodeById(const NgId id) const {
  IdMap::const_iterator iter;
  
  if ((iter = idMap.find(id)) != idMap.end())
    return iter->second.node;
  else
    return nullptr;
}

// set "id"-attribute to some new id
void IdPool::addIdedObj(IdedObj *n) {
  IdMap::iterator iter = IdAllocAnon::idAlloc(IdMapEntry(n));
  setId(n, iter->first);
}

// set "id"-attribute to id
void IdPool::addIdedObj(NamedIdedObj *n) {
  CoSupport::Math::FNV<IdRangeNamed::bits> hf;
  NgId id = IdRangeNamed::min + hf(getName(n));
  assert(id <= IdRangeNamed::max);
  IdMap::iterator iter = IdAllocNamed::idAllocNext(id, IdMapEntry(n));
  setId(n, iter->first);
}

void IdPool::addIdedObj(const NgId id, IdedObj *n) {
  sassert(idMap.insert(std::make_pair(id, n)).second);
  setId(n, id);
}

} } // namespace smoc::Detail

#endif // SYSTEMOC_NEED_IDS
