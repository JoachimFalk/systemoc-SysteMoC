// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_IDEDOBJ_HPP
#define _INCLUDED_SMOC_DETAIL_IDEDOBJ_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_NEED_IDS

#include <limits>
#include <cstdint> // For uint32_t
#include <cassert>

namespace smoc { namespace Detail {

// This must be synced with SystemCoDesigner::SGX::NgId
typedef uint32_t NgId;

class IdedObjAccess;

class IdedObj {
  friend class IdedObjAccess;
private:
  NgId _id;
protected:
  IdedObj()
    : _id(std::numeric_limits<NgId>::max()) {}

  // Make class polymorphic in order to make typeid derive correct types for derived instances.
  virtual ~IdedObj()
    {}

  NgId getId() const
    { return _id; }
};

class IdedObjAccess {
public:
  static
  NgId getId(IdedObj const *idedObj)
    { return idedObj->_id; }
  static
  void setId(IdedObj *idedObj, NgId id) {
    assert(idedObj->_id == std::numeric_limits<NgId>::max());
    idedObj->_id = id;
  }
};

} } // namespace smoc::Detail

#endif // SYSTEMOC_NEED_IDS

#endif /* _INCLUDED_SMOC_DETAIL_IDEDOBJ_HPP */
