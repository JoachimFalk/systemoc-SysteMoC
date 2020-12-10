// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Martin Letras <martin.letras@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_REGISTERCHAN_HPP
#define _INCLUDED_SMOC_DETAIL_REGISTERCHAN_HPP

#include "RegisterStorage.hpp"
#include "RegisterEntry.hpp"
#include "RegisterOutlet.hpp"

namespace smoc { namespace Detail {

template <typename T>
class RegisterChan
  : public RegisterStorage<T>
{
  typedef RegisterChan<T>   this_type;
public:
  typedef T                 data_type;
  typedef RegisterOutlet<T> outlet_type;
  typedef RegisterEntry<T>  entry_type;

  /// @brief Constructor
  RegisterChan(typename RegisterStorage<T>::chan_init const &i)
    : RegisterStorage<T>(i) {}
protected:
  /// @brief See smoc_port_registry
  smoc::Detail::PortOutBaseIf *createEntry()
    { return new entry_type(this); }

  /// @brief See smoc_port_registry
  smoc::Detail::PortInBaseIf *createOutlet()
    { return new outlet_type(this); }
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_REGISTERCHAN_HPP */
