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

#ifndef _INCLUDED_SMOC_DETAIL_REGISTERENTRY_HPP
#define _INCLUDED_SMOC_DETAIL_REGISTERENTRY_HPP

#include "Storage.hpp"
#include "RegisterChanBase.hpp"
#include "RegisterStorage.hpp"

#include "../smoc_event.hpp"
#include "../../systemoc/detail/smoc_chan_if.hpp"

namespace smoc { namespace Detail {

template <typename T> class RegisterChan;

class RegisterEntryBase {
protected:
  RegisterEntryBase(RegisterChanBase *chan);

  RegisterChanBase *chan;
  smoc_event        trueEvent;
};

template <typename T>
class RegisterEntry
  : public RegisterEntryBase
  , public smoc_port_out_if<T>
  , public smoc_port_out_if<T>::access_type
{
  typedef RegisterEntry<T>                      this_type;
public:
  typedef T                                     data_type;
  typedef Storage<T>                            storage_type;
  typedef typename this_type::access_out_type   ring_out_type;
  typedef typename this_type::return_type       return_type;
  typedef smoc_port_out_if<T>                   iface_type;

  /// @brief Constructor
  RegisterEntry(RegisterChan<T> *chan)
    : RegisterEntryBase(chan) {}

  // Interfaces independent of T.

  /// @brief See PortOutBaseIf
  const char *name() const
    { return chan->name();}

  /// @brief See PortOutBaseIf
  size_t numFree() const
    { return 1; }

  // Interfaces depending on T.

  /// @brief See smoc_1d_port_access_if
  return_type operator[](size_t n)
    { assert(n == 0); return static_cast<RegisterStorage<T> *>(chan)->actualValue; }

  /// @brief See smoc_1d_port_access_if
  const return_type operator[](size_t n) const
    { assert(n == 0); return static_cast<RegisterStorage<T> *>(chan)->actualValue; }
protected:

  /// @brief See smoc_port_out_if
  ring_out_type *getWritePortAccess()
    { return this; }

  // Interfaces independent of T.

  /// @brief See PortBaseIf
  void commStart(size_t produce) {
  }
  /// @brief See PortBaseIf
  void commFinish(size_t produce, bool dropped = false) {
    assert(!dropped);
  }

  /// @brief See PortBaseIf
  void commExec(size_t produce) {
    commStart(produce);
    commFinish(produce);
  }

  /// @brief See PortOutBaseIf
  smoc::smoc_event &spaceAvailableEvent(size_t n) {
    assert(n <= 1);
    return trueEvent;
  }

///// @brief See PortOutBaseIf
//size_t outTokenId() const
//  { return chan->outTokenId(); }

  /// @brief See PortOutBaseIf
  void moreSpace(size_t) {}

  /// @brief See PortOutBaseIf
  void lessSpace(size_t) {}

  /// @brief See PortBaseIf::AccessIf
  bool tokenIsValid(size_t n) const
    { assert(n == 0); return static_cast<RegisterStorage<T> *>(chan)->isValid(); }

  /// @brief See PortBaseIf::AccessIf
  void setLimit(size_t l) {}

  // Interfaces depending on T.
};

} } // namespace smoc

#endif /* _INCLUDED_SMOC_DETAIL_REGISTERENTRY_HPP */
