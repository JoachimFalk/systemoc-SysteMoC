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

#ifndef _INCLUDED_SMOC_DETAIL_NAMEDIDEDOBJ_HPP
#define _INCLUDED_SMOC_DETAIL_NAMEDIDEDOBJ_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_NEED_IDS

#include "IdedObj.hpp"

namespace smoc { namespace Detail {

class NamedIdedObjAccess;

class NamedIdedObj: public IdedObj {
  friend class NamedIdedObjAccess;
protected:
  virtual const char *name() const = 0;
  virtual ~NamedIdedObj() {}
};

class NamedIdedObjAccess: public IdedObjAccess {
public:
  static
  const char *getName(NamedIdedObj const *namedIdedObj)
    { return namedIdedObj->name(); }
};

} } // namespace smoc::Detail

#endif // SYSTEMOC_NEED_IDS

#endif /* _INCLUDED_SMOC_DETAIL_NAMEDIDEDOBJ_HPP */
