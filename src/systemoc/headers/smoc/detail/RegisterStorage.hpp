// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
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

#ifndef _INCLUDED_SMOC_DETAIL_REGISTERSTORAGE_HPP
#define _INCLUDED_SMOC_DETAIL_REGISTERSTORAGE_HPP

#include "RegisterChanBase.hpp"
#include "Storage.hpp"

#include <string>
#include <vector>

#include <cstddef>

namespace smoc { namespace Detail {

template <typename T> class RegisterOutlet;
template <typename T> class RegisterEntry;

template <typename T>
class RegisterStorage
  : public RegisterChanBase
{
  typedef RegisterStorage<T> this_type;

  friend class RegisterOutlet<T>;
  friend class RegisterEntry<T>;
public:

  typedef Storage<T>         storage_type;

  /// @brief Channel initializer
  class chan_init
    : public RegisterChanBase::chan_init {
    friend class RegisterStorage<T>;
  private:
    //FIXME(MS): replace with signal value wrapper
    std::vector<T>  marking;
  public:
    void add(T const &x) {
      if (chan)
        assert(!"Can't place initial token: Channel already created!");
      assert(marking.empty());
      marking.push_back(x);
    }

    chan_init &operator<<(T const &x) {
      this->add(x);
      return *this;
    }
  protected:
    chan_init(std::string const &name, size_t n)
      : RegisterChanBase::chan_init(name, n) {}
  };
protected:
  /// @brief Constructor
  RegisterStorage(chan_init const &i)
    : RegisterChanBase(i)
  {
    assert(i.marking.size() <= 1);
    if (i.marking.size() > 0)
      actualValue.put(i.marking[0]);
  }

  bool isValid() const
    { return actualValue.isValid(); }

  void setChannelID( std::string sourceActor,
                             CoSupport::SystemC::ChannelId id,
                             std::string name ){
    actualValue.setChannelID(sourceActor, id, name);
  }

  /// @See smoc_root_chan
  virtual void doReset() {
    RegisterChanBase::doReset();
    this->actualValue.invalidate();
  }

  storage_type actualValue;
};

template <>
class RegisterStorage<void>
  : public RegisterChanBase
{
  typedef RegisterStorage<void> this_type;
  typedef RegisterChanBase      base_type;

  friend class RegisterOutlet<void>;
  friend class RegisterEntry<void>;
public:

  typedef void                  storage_type;

protected:
  /// @brief Constructor
  RegisterStorage(chan_init const &i)
    : RegisterChanBase(i) {}

  bool isValid() const
    { return true; }

  /// @See smoc_root_chan
  virtual void doReset() {
    RegisterChanBase::doReset();
  }
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_REGISTERSTORAGE_HPP */
