// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Christian Zebelein <christian.zebelein@fau.de>
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Sebastian Graf <sebastian.graf@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2014 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Rafael Rosales <rafael.rosales@fau.de>
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

#ifndef _INCLUDED_SMOC_SMOC_FIFO_HPP
#define _INCLUDED_SMOC_SMOC_FIFO_HPP

#include "detail/FifoChan.hpp"
#include "detail/ConnectProvider.hpp"

#include <systemoc/smoc_config.h>

#include <CoSupport/commondefs.h>

#include <systemc>

namespace smoc {

/**
 * This class is the channel initializer for Detail::FifoChan
 */
template <typename T>
class smoc_fifo
  : public Detail::FifoChan<T>::chan_init
  , public Detail::ConnectProvider<
      smoc_fifo<T>,
      Detail::FifoChan<T> >
{
public:
  //typedef T                 data_type;
  typedef smoc_fifo<T>                  this_type;
  typedef typename this_type::chan_type chan_type;
  typedef typename chan_type::chan_init base_type;
  friend typename this_type::con_type;
  friend class smoc_reset_net;
private:
  chan_type *chan;
public:
  /// @brief Constructor
  smoc_fifo(size_t n = 1)
    : base_type("", n), chan(nullptr)
  {}

  /// @brief Constructor
  explicit smoc_fifo(const std::string& name, size_t n = 1)
    : base_type(name, n), chan(nullptr)
  {}

  /// @brief Constructor
  smoc_fifo(const this_type &x)
    : base_type(x), chan(nullptr)
  {
    if(x.chan)
      assert(!"Can't copy initializer: Channel already created!");
  }

  this_type &operator<<(typename this_type::add_param_ty x) {
    if(chan)
      assert(!"Can't place initial token: Channel already created!");
    this->add(x);
    return *this;
  }
  
private:
  chan_type *getChan() {
    if (chan == nullptr)
      chan = new chan_type(*this);
    return chan;
  }

  // disable
  this_type &operator =(const this_type &);
};

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_FIFO_HPP */
