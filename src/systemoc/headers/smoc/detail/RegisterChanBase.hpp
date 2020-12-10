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

#ifndef _INCLUDED_SMOC_DETAIL_REGISTERCHANBASE_HPP
#define _INCLUDED_SMOC_DETAIL_REGISTERCHANBASE_HPP

#include "ChanBase.hpp"

#include <boost/noncopyable.hpp>

#include <string>
#include <cstddef>

namespace smoc { namespace Detail {

/// Base class for the smoc_register channel implementation.
class RegisterChanBase
  : public ChanBase
  , private boost::noncopyable
{
  typedef RegisterChanBase  this_type;
public:
  friend class smoc_register_outlet_base;
  friend class smoc_register_entry_base;

  /// @brief Channel initializer
  class chan_init {
  public:
    friend class RegisterChanBase;
  protected:
    chan_init(const std::string &name, size_t n);

    RegisterChanBase *chan;
  private:
    std::string name;
    size_t      n;
  };

protected:
  // constructors
  RegisterChanBase(chan_init const &i, size_t tokenSize);

  size_t inTokenId() const
    { return tokenId; }

  size_t outTokenId() const
    { return tokenId; }
private:
  /// @brief The tokenId of the next commit token
  size_t tokenId;
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_REGISTERCHANBASE_HPP */
