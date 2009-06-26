// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_NEED_IDS

#include <smoc/detail/IdPool.hpp>

#include <CoSupport/Math/string_hash.hpp>

namespace SysteMoC { namespace Detail {

IdedObj *IdPool::getNodeById(const NgId id) const {
  IdMap::const_iterator iter;
  
  if ((iter = idMap.find(id)) != idMap.end())
    return iter->second.node;
  else
    return NULL;
}

// set "id"-attribute to some new id
void IdPool::addIdedObj(IdedObj *n) {
  IdMap::iterator iter = IdAllocAnon::idAlloc(IdMapEntry(n));
  n->setId(iter->first);
}

// set "id"-attribute to id
void IdPool::addIdedObj(NamedIdedObj *n) {
  CoSupport::FNV<IdRangeNamed::bits> hf;
  NgId id = IdRangeNamed::min + hf(n->name());
  assert(id <= IdRangeNamed::max);
  IdMap::iterator iter = IdAllocNamed::idAllocNext(id, IdMapEntry(n));
  n->setId(iter->first);
}

} } // namespace SysteMoC::Detail

#endif // SYSTEMOC_NEED_IDS
