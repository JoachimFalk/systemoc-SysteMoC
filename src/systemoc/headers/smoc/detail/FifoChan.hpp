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

#ifndef _INCLUDED_SMOC_DETAIL_FIFOCHAN_HPP
#define _INCLUDED_SMOC_DETAIL_FIFOCHAN_HPP

#include "FifoStorage.hpp"
#include "FifoChanBase.hpp"
#include "FifoEntry.hpp"
#include "FifoOutlet.hpp"

#include "../smoc_token_traits.hpp"

namespace smoc { namespace Detail {

/**
 * This class provides interfaces and connect methods
 */
template<class T>
class FifoChan
: public FifoStorage<T, FifoChanBase>
{
  friend class FifoOutlet<T>;
  friend class FifoEntry<T>;
public:
  typedef T                           data_type;
  typedef FifoChan<data_type>   this_type;
  typedef FifoEntry<data_type>  entry_type;
  typedef FifoOutlet<data_type> outlet_type;
  
  /// @brief Channel initializer
  typedef typename FifoStorage<T, FifoChanBase>::chan_init chan_init;

  /// @brief Constructor
  FifoChan(const chan_init &i)
    : FifoStorage<T, FifoChanBase>(i, smoc_token_traits<T>::tokenSize())
  {}
protected:
  /// @brief See smoc_port_registry
  PortOutBaseIf *createEntry()
    { return new entry_type(*this); }

  /// @brief See smoc_port_registry
  PortInBaseIf *createOutlet()
    { return new outlet_type(*this); }

  class InvalidateTokenGenerator {
    int n;
  public:
    InvalidateTokenGenerator(int n): n(n) {}

    int popMax() { return --n; }
    int count() const { return n; }
  };

#ifdef SYSTEMOC_ENABLE_ROUTING
  void invalidateToken(size_t x) {
    this->dropRInvisible(InvalidateTokenGenerator(x));
  }
#endif //defined(SYSTEMOC_ENABLE_ROUTING)

private:
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_FIFOCHAN_HPP */
