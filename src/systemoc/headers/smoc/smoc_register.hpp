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

#ifndef _INCLUDED_SMOC_SMOC_REGISTER_HPP
#define _INCLUDED_SMOC_SMOC_REGISTER_HPP

#include "detail/ConnectProvider.hpp"
#include "detail/RegisterChan.hpp"

#include <boost/noncopyable.hpp>

namespace smoc {

template <typename T>
class smoc_register
  : public Detail::RegisterChan<T>::chan_init
  , public Detail::ConnectProvider<
      smoc_register<T>,
      Detail::RegisterChan<T> >
{
  typedef smoc_register<T>                            this_type;
  typedef typename Detail::RegisterChan<T>::chan_init base_type;

  friend class Detail::ConnectProvider<this_type, typename this_type::chan_type>;
public:
  typedef T                             data_type;
  typedef typename this_type::chan_type chan_type;
public:
  /// @brief Constructor
  smoc_register()
    : base_type("", 1)
  {}

  /// @brief Constructor
  explicit smoc_register(const std::string &name)
    : base_type(name, 1)
  {}

  /// @brief Constructor
  smoc_register(const this_type &x)
    : base_type(x)
  {
    if(x.chan)
      assert(!"Can't copy initializer: Channel already created!");
  }

private:
  chan_type *getChan() {
    if (this->chan == nullptr)
      this->chan = new chan_type(*this);
    return static_cast<chan_type *>(this->chan);
  }

  // disable
  this_type &operator =(const this_type &);
};

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_REGISTER_HPP */
