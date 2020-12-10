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

#ifndef _INCLUDED_SMOC_DETAIL_REGISTEROUTLET_HPP
#define _INCLUDED_SMOC_DETAIL_REGISTEROUTLET_HPP

#include "RegisterChanBase.hpp"
#include "Storage.hpp"

#include "../smoc_event.hpp"
#include "../../systemoc/detail/smoc_chan_if.hpp"

#include <cstddef>
#include <cassert>

namespace smoc { namespace Detail {

template <typename T> class RegisterChan;

class RegisterOutletBase {
protected:
  RegisterOutletBase(RegisterChanBase *chan);

  RegisterChanBase *chan;
  smoc_event        trueEvent;
};

template <typename T>
class RegisterOutlet
  : public RegisterOutletBase
  , public smoc_port_in_if<T>
  , public smoc_port_in_if<T>::access_type
{
  typedef RegisterOutlet<T>                     this_type;
public:
  typedef T                                     data_type;
  typedef Storage<T>                            storage_type;
  typedef typename this_type::access_in_type    ring_in_type;
  typedef typename this_type::return_type       return_type;
  typedef smoc_port_in_if<T>                    iface_type;

  /// @brief Constructor
  RegisterOutlet(RegisterChan<T> *chan)
    : RegisterOutletBase(chan) {}

  // Interfaces independent of T.

  /// @brief See PortInBaseIf
  const char *name() const
    { return chan->name();}

  /// @brief See PortInBaseIf
  size_t numAvailable() const
    { return 1; }

  // Interfaces depending on T.

  /// @brief See smoc_1d_port_access_if
  return_type operator[](size_t n)
    { assert(n == 0); return static_cast<RegisterChan<T> *>(chan)->actualValue; }

  /// @brief See smoc_1d_port_access_if
  const return_type operator[](size_t n) const
    { assert(n == 0); return static_cast<RegisterChan<T> *>(chan)->actualValue; }

protected:
  /// @brief See smoc_port_in_if
  ring_in_type *getReadPortAccess()
    { return this; }

  // Interfaces independent of T.

  /// @brief See PortBaseIf
  void commStart(size_t consume) {
  }
  /// @brief See PortBaseIf
  void commFinish(size_t consume, bool dropped = false) {
    assert(!dropped);
  }

  /// @brief See PortBaseIf
  void commExec(size_t consume) {
    commStart(consume);
    commFinish(consume);
  }

  /// @brief See PortInBaseIf
  smoc_event &dataAvailableEvent(size_t n) {
    assert(n <= 1);
    return trueEvent;
  }

  /// @brief See PortInBaseIf
  void moreData(size_t) {}

  /// @brief See PortInBaseIf
  void lessData(size_t) {}

///// @brief See PortInBaseIf
//size_t inTokenId() const
//  { return chan->inTokenId(); }

  /// @brief See smoc_1d_port_access_if
  bool tokenIsValid(size_t n) const
    { assert(n == 0); return static_cast<RegisterChan<T> *>(chan)->isValid(); }

  /// @brief See smoc_1d_port_access_if
  void setLimit(size_t l) {}

};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_REGISTEROUTLET_HPP */
