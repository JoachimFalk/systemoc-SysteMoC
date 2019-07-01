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

#ifndef _INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP
#define _INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP

#include "PortBaseIf.hpp"
#include "../smoc_guard.hpp"
#include "../SimulatorAPI/ChannelInterfaces.hpp"

template<class, class> class smoc_chan_adapter;

namespace smoc { namespace Detail {

template<class IFaceImpl>
class ChanAdapterMid;

class PortInBaseIf
  : public SimulatorAPI::ChannelSourceInterface
  , public PortBaseIf
{
  typedef PortInBaseIf this_type;

  friend class PortInBase;
  template<class,class> friend class ::smoc_chan_adapter;
  template<class>       friend class ChanAdapterMid;
public:
  // FIXME: Why not merge this with PortInBaseIf?!
  class access_type {
  public:
    typedef void return_type;

  #if defined(SYSTEMOC_ENABLE_DEBUG)
    virtual void setLimit(size_t) = 0;
  #endif
    virtual bool tokenIsValid(size_t) const = 0;

    virtual ~access_type() {}
  };

//virtual size_t      inTokenId() const = 0;
  virtual size_t      numAvailable() const = 0;
  virtual const char *name() const = 0;

  virtual ~PortInBaseIf() {}
protected:
  // constructor
  PortInBaseIf() {}
 
  virtual smoc_event &dataAvailableEvent(size_t n) = 0;

  smoc_event &blockEvent(size_t n)
    { return this->dataAvailableEvent(n); }  
  size_t availableCount() const
    { return this->numAvailable(); }

#ifdef SYSTEMOC_ENABLE_ROUTING
  virtual void commStart(size_t n) = 0;
  virtual void commFinish(size_t n, bool dropped = false) = 0;
  virtual void commExec(size_t n) {
    commStart(n); commFinish(n);
  }
#else //!SYSTEMOC_ENABLE_ROUTING
  virtual void commExec(size_t n) = 0;
#endif //!SYSTEMOC_ENABLE_ROUTING

  /// @brief Reset
  virtual void reset() {}

  virtual access_type *getChannelAccess() = 0;
};

class PortOutBaseIf
  // SimulatorAPI::ChannelSourceInterface must be first base class. Otherwise,
  // the reinterpret_cast<std::vector<SimulatorAPI::ChannelSinkInterface *>
  //   (std::vector<PortOutBaseIf *>()) in PortOutBase is invalid!!!
  : public SimulatorAPI::ChannelSinkInterface
  , public PortBaseIf
{
  typedef PortOutBaseIf this_type;

  friend class PortOutBase;
  template<class,class> friend class ::smoc_chan_adapter;
  template<class>       friend class ChanAdapterMid;
public:
  // FIXME: Why not merge this with PortOutBaseIf?!
  class access_type {
  public:
    typedef void return_type;

  #if defined(SYSTEMOC_ENABLE_DEBUG)
    virtual void setLimit(size_t) = 0;
  #endif
    virtual bool tokenIsValid(size_t) const = 0;

    virtual ~access_type() {}
  };

//virtual size_t      outTokenId() const = 0;
  virtual size_t      numFree() const = 0;
  virtual const char *name() const = 0;

  virtual ~PortOutBaseIf() {}
protected:
  // constructor
  PortOutBaseIf() {}

  virtual smoc_event &spaceAvailableEvent(size_t n) = 0;

  smoc_event &blockEvent(size_t n)
    { return this->spaceAvailableEvent(n); }  
  size_t availableCount() const
    { return this->numFree(); }

#ifdef SYSTEMOC_ENABLE_ROUTING
  virtual void commStart(size_t n) = 0;
  virtual void commFinish(size_t n, bool dropped = false) = 0;
  virtual void commExec(size_t n) {
    commStart(n); commFinish(n);
  }
#else //!SYSTEMOC_ENABLE_ROUTING
  virtual void commExec(size_t n) = 0;
#endif //!SYSTEMOC_ENABLE_ROUTING

  /// @brief Reset
  virtual void reset() {}

  virtual access_type *getChannelAccess() = 0;
};

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_PORTIOBASEIF_HPP */
