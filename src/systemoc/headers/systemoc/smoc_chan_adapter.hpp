// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SYSTEMOC_SMOC_CHAN_ADAPTER_HPP
#define _INCLUDED_SYSTEMOC_SMOC_CHAN_ADAPTER_HPP

#include "../smoc/detail/PortBaseIf.hpp"
#include "../smoc/smoc_event.hpp"

#include "detail/smoc_chan_if.hpp"

#include <systemoc/smoc_config.h>

#include <tlm.h>
#include <systemc>

#include <boost/type_traits/is_base_of.hpp>

namespace tlm {

/**
 * Specializations of tlm interfaces for void 
 * (can't use (const) reference to void as parameter!)
 */

// get interfaces
template<>
class tlm_blocking_get_if<void> : public virtual sc_core::sc_interface {
public:
  virtual void get(tlm_tag<void> * = nullptr) = 0; // can't use "void&"
};

template<>
class tlm_nonblocking_get_if<void> : public virtual sc_core::sc_interface {
public:
  virtual bool nb_get(tlm_tag<void> * = nullptr) = 0; // can't use "void&"
  virtual bool nb_can_get(tlm_tag<void> * = nullptr) const = 0;
  virtual const sc_core::sc_event &ok_to_get(tlm_tag<void> * = nullptr) const = 0;
};

// peek interfaces
template<>
class tlm_blocking_peek_if<void> : public virtual sc_core::sc_interface {
public:
  virtual void peek(tlm_tag<void> * = nullptr) const = 0;
};

template<>
class tlm_nonblocking_peek_if<void> : public virtual sc_core::sc_interface {
public:
  virtual bool nb_peek(tlm_tag<void> * = nullptr) const = 0;
  virtual bool nb_can_peek(tlm_tag<void> * = nullptr) const = 0;
  virtual const sc_core::sc_event &ok_to_peek(tlm_tag<void> * = nullptr) const = 0;
};

// put interfaces
template<>
class tlm_blocking_put_if<void> : public virtual sc_core::sc_interface {
public:
  virtual void put(tlm_tag<void> * = nullptr) = 0; // can't use "const void&"
};

template<>
class tlm_nonblocking_put_if<void> : public virtual sc_core::sc_interface {
public:
  virtual bool nb_put(tlm_tag<void> * = nullptr) = 0; // can't use "const void&"
  virtual bool nb_can_put(tlm_tag<void> * = nullptr) const = 0;
  virtual const sc_core::sc_event &ok_to_put(tlm_tag<void> * = nullptr) const = 0;
};

} // namespace tlm

namespace smoc { namespace Detail {

  template <typename DERIVED, typename CHANTYPE>
  class ConnectProvider;

  class ChanAdapterBase
  : public virtual sc_core::sc_interface {
  private:
    PortBaseIf &iface;
  protected:
    ChanAdapterBase(PortBaseIf &iface)
      : iface(iface) {}
  public:
    PortBaseIf       &getIface()
      { return iface; }
    PortBaseIf const &getIface() const
      { return iface; }
  };

  template<class IFaceImpl>
  class ChanAdapterMid;

  template<class T>
  class ChanAdapterMid<smoc_port_in_if<T> >
  : public ChanAdapterBase {
    typedef ChanAdapterMid<smoc_port_in_if<T> >   this_type;
    typedef ChanAdapterBase                       base_type;
  protected:
    /// typedefs
    typedef smoc_port_in_if<T> iface_impl_type;

    smoc_event &dataAvailable;

    iface_impl_type       &getIface()
      { return static_cast<iface_impl_type &>(base_type::getIface()); }
    iface_impl_type const &getIface() const
      { return static_cast<iface_impl_type const &>(base_type::getIface()); }
  public:
    /// constructor
    /// - stores reference to wrapped interface
    /// - needs read channel access
    ChanAdapterMid(iface_impl_type &in_if)
      : base_type(in_if),
        dataAvailable(in_if.dataAvailableEvent(1)) {}
  protected:
    /// @brief See sc_interface
    void register_port(sc_core::sc_port_base &p, const char *if_ty)
      { getIface().register_port(p, typeid(iface_impl_type).name()); }
  };

  template<class T>
  class ChanAdapterMid<smoc_port_out_if<T> >
  : public ChanAdapterBase {
    typedef ChanAdapterMid<smoc_port_out_if<T> > this_type;
    typedef ChanAdapterBase                      base_type;
  protected:
    /// typedefs
    typedef smoc_port_out_if<T> iface_impl_type;

    smoc_event &spaceAvailable;

    iface_impl_type       &getIface()
      { return static_cast<iface_impl_type &>(base_type::getIface()); }
    iface_impl_type const &getIface() const
      { return static_cast<iface_impl_type const &>(base_type::getIface()); }
  public:
    /// constructor
    /// - stores reference to wrapped interface
    /// - needs write channel access
    ChanAdapterMid(iface_impl_type &out_if)
      : base_type(out_if),
        spaceAvailable(out_if.spaceAvailableEvent(1)) {}
  protected:
    /// @brief See sc_interface
    void register_port(sc_core::sc_port_base &p, const char *if_ty)
      { getIface().register_port(p, typeid(iface_impl_type).name()); }
  };

} } // namespace smoc::Detail


/**
 * specialize this class to obtain an adapter from IFace to IFaceImpl
 * - derive class implements IFace and Detail::ChanAdapterMid
 * - provide an instance of IFaceImpl (it's an adapter, it needs not
 *   implement IFaceImpl)
 * - set isAdapter to true
 */
template<class IFaceImpl, class IFace>
class smoc_chan_adapter {
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  /// flag if this class is a specialization
  static const bool isAdapter = false;
};

/**
 * adapter specialization for blocking tlm get -> smoc channel read
 */
template<class T>
class smoc_chan_adapter<
    smoc_port_in_if<T>,
    tlm::tlm_blocking_get_if<T> >
  : public tlm::tlm_blocking_get_if<T>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_in_if<T> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<T> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type &in_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_in_if<T> >(in_if) {}

  /// see tlm::tlm_blocking_get_if<T>
  T get(tlm::tlm_tag<T> * = nullptr) {
    smoc::smoc_wait(this->dataAvailable);
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    const T &t = (*ca)[0];
    
    this->getIface().commExec(1);
    return t;
  }
};

/**
 * adapter specialization for blocking tlm get -> smoc channel read
 */
template<>
class smoc_chan_adapter<
    smoc_port_in_if<void>,
    tlm::tlm_blocking_get_if<void> >
  : public tlm::tlm_blocking_get_if<void>,
    public smoc::Detail::ChanAdapterMid
      <smoc_port_in_if<void> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<void> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type &in_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_in_if<void> >(in_if) {}
  /// see tlm::tlm_blocking_get_if<void>
  void get(tlm::tlm_tag<void> * = nullptr) {
    smoc::smoc_wait(this->dataAvailable);
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    this->getIface().commExec(1);
  }
};

/**
 * adapter specialization for nonblocking tlm get -> smoc channel read
 */
template<class T>
class smoc_chan_adapter<
    smoc_port_in_if<T>,
    tlm::tlm_nonblocking_get_if<T> >
  : public tlm::tlm_nonblocking_get_if<T>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_in_if<T> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<T> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type &in_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_in_if<T> >(in_if),
      scev(this->dataAvailable) {}
  /// see tlm_nonblocking_get_if<T>
  bool nb_get(T &t) {
    if (!this->dataAvailable)
      return false;
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    t = (*ca)[0];
    
    this->getIface().commExec(1);
    
    return true;
  }
  
  /// see tlm_nonblocking_get_if<T>
  bool nb_can_get(tlm::tlm_tag<T> * = nullptr) const
    { return this->dataAvailable; }

  /// see tlm_nonblocking_get_if<T>
  const sc_core::sc_event& ok_to_get(tlm::tlm_tag<T> * = nullptr) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for nonblocking tlm get -> smoc channel read
 */
template<>
class smoc_chan_adapter<
    smoc_port_in_if<void>,
    tlm::tlm_nonblocking_get_if<void> >
  : public tlm::tlm_nonblocking_get_if<void>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_in_if<void> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<void> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type &in_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_in_if<void> >(in_if),
      scev(this->dataAvailable) {}

  /// see tlm_nonblocking_get_if<void>
  bool nb_get(tlm::tlm_tag<void> * = nullptr) {
    if (!this->dataAvailable)
      return false;
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
    this->getIface().commExec(1);
    
    return true;
  }

  /// see tlm_nonblocking_get_if<void>
  bool nb_can_get(tlm::tlm_tag<void> * = nullptr) const
    { return this->dataAvailable; }

  /// see tlm_nonblocking_get_if<void>
  const sc_core::sc_event& ok_to_get(tlm::tlm_tag<void> * = nullptr) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for blocking tlm peek -> smoc channel read
 */
template<class T>
class smoc_chan_adapter<
    smoc_port_in_if<T>,
    tlm::tlm_blocking_peek_if<T> >
  : public tlm::tlm_blocking_peek_if<T>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_in_if<T> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<T>                    iface_impl_type;
  typedef smoc_chan_adapter<iface_impl_type,
      tlm::tlm_blocking_peek_if<T> >            this_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type &in_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_in_if<T> >(in_if) {}

  T peek(tlm::tlm_tag<T> *t = nullptr) const {
    smoc::smoc_wait(this->dataAvailable);
    
    typename iface_impl_type::access_type *ca =
      const_cast<this_type *>(this)->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    return (*ca)[0];
  }
};

/**
 * adapter specialization for blocking tlm peek -> smoc channel read
 */
template<>
class smoc_chan_adapter<
    smoc_port_in_if<void>,
    tlm::tlm_blocking_peek_if<void> >
  : public tlm::tlm_blocking_peek_if<void>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_in_if<void> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<void>                 iface_impl_type;
  typedef smoc_chan_adapter<
      iface_impl_type,
      tlm::tlm_blocking_peek_if<void> >         this_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type &in_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_in_if<void> >(in_if) {}

  void peek(tlm::tlm_tag<void> *t = nullptr) const {
    smoc::smoc_wait(this->dataAvailable);
    
    typename iface_impl_type::access_type *ca =
      const_cast<this_type *>(this)->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    return;
  }
};

/**
 * adapter specialization for nonblocking tlm peek -> smoc channel read
 */
template<class T>
class smoc_chan_adapter<
    smoc_port_in_if<T>,
    tlm::tlm_nonblocking_peek_if<T> >
  : public tlm::tlm_nonblocking_peek_if<T>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_in_if<T> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<T> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type &in_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_in_if<T> >(in_if),
      scev(this->dataAvailable) {}

  /// see tlm_nonblocking_peek_if<T>
  bool nb_peek(T &t) {
    if (!this->dataAvailable)
      return false;
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    t = (*ca)[0];
    
    return true;
  }

  /// see tlm_nonblocking_peek_if<T>
  bool nb_can_peek(tlm::tlm_tag<T> * = nullptr) const
    { return this->dataAvailable; }

  /// see tlm_nonblocking_peek_if<T>
  const sc_core::sc_event& ok_to_peek(tlm::tlm_tag<T> * = nullptr) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for nonblocking tlm peek -> smoc channel read
 */
template<>
class smoc_chan_adapter<
    smoc_port_in_if<void>,
    tlm::tlm_nonblocking_peek_if<void> >
  : public tlm::tlm_nonblocking_peek_if<void>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_in_if<void> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<void> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type &in_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_in_if<void> >(in_if),
      scev(this->dataAvailable) {}

  /// see tlm_nonblocking_peek_if<void>
  bool nb_peek(tlm::tlm_tag<void> * = nullptr) {
    if (!this->dataAvailable)
      return false;
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
    return true;
  }

  /// see tlm_nonblocking_peek_if<void>
  bool nb_can_peek(tlm::tlm_tag<void> * = nullptr) const
    { return this->dataAvailable; }

  /// see tlm_nonblocking_peek_if<void>
  const sc_core::sc_event& ok_to_peek(tlm::tlm_tag<void> * = nullptr) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for blocking tlm put -> smoc channel write
 */
template<class T>
class smoc_chan_adapter<
    smoc_port_out_if<T>,
    tlm::tlm_blocking_put_if<T> >
  : public tlm::tlm_blocking_put_if<T>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_out_if<T> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_out_if<T> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type &out_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_out_if<T> >(out_if) {}

  /// see tlm::tlm_blocking_put_if<T>
  void put(const T &t) {
    smoc::smoc_wait(this->spaceAvailable);
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    (*ca)[0] = t;
    
    this->getIface().commExec(1);
  }
};

/**
 * adapter specialization for blocking tlm put -> smoc channel write
 */
template<>
class smoc_chan_adapter<
    smoc_port_out_if<void>,
    tlm::tlm_blocking_put_if<void> >
  : public tlm::tlm_blocking_put_if<void>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_out_if<void> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_out_if<void> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type &out_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_out_if<void> >(out_if) {}

  /// see tlm::tlm_blocking_put_if<void>
  void put(tlm::tlm_tag<void> * = nullptr) {
    smoc::smoc_wait(this->spaceAvailable);
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
    this->getIface().commExec(1);
  }
};

/**
 * adapter specialization for nonblocking tlm put -> smoc channel write
 */
template<class T>
class smoc_chan_adapter<
    smoc_port_out_if<T>,
    tlm::tlm_nonblocking_put_if<T> >
  : public tlm::tlm_nonblocking_put_if<T>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_out_if<T> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_out_if<T> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type &out_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_out_if<T> >(out_if),
      scev(this->spaceAvailable) {}

  /// see tlm::tlm_nonblocking_put_if<T>
  bool nb_put(const T &t) {
    if (!this->spaceAvailable)
      return false;
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    (*ca)[0] = t;
    
    this->getIface().commExec(1);
    
    return true;
  }

  /// see tlm::tlm_nonblocking_put_if<T>
  bool nb_can_put(tlm::tlm_tag<T> * = nullptr) const
    { return this->spaceAvailable; }

  /// see tlm::tlm_nonblocking_put_if<T>
  const sc_core::sc_event &ok_to_put(tlm::tlm_tag<T> * = nullptr) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for nonblocking tlm put -> smoc channel write
 */
template<>
class smoc_chan_adapter<
    smoc_port_out_if<void>,
    tlm::tlm_nonblocking_put_if<void> >
  : public tlm::tlm_nonblocking_put_if<void>
  , public smoc::Detail::ChanAdapterMid
      <smoc_port_out_if<void> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_out_if<void> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type &out_if)
    : smoc::Detail::ChanAdapterMid<smoc_port_out_if<void> >(out_if),
      scev(this->spaceAvailable) {}

  /// see tlm::tlm_nonblocking_put_if<void>
  bool nb_put(tlm::tlm_tag<void> * = nullptr) {
    if (!this->spaceAvailable)
      return false;
    
    typename iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
    this->getIface().commExec(1);
    
    return true;
  }

  /// see tlm::tlm_nonblocking_put_if<void>
  bool nb_can_put(tlm::tlm_tag<void> * = nullptr) const
    { return this->spaceAvailable; }

  /// see tlm::tlm_nonblocking_put_if<void>
  const sc_core::sc_event &ok_to_put(tlm::tlm_tag<void> * = nullptr) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for nonblocking tlm get and peek -> smoc channel read
 */
template<class T>
class smoc_chan_adapter<
    smoc_port_in_if<T>,
    tlm::tlm_nonblocking_get_peek_if<T> >
  : public smoc_chan_adapter<smoc_port_in_if<T>,
      tlm::tlm_nonblocking_get_if<T> >
  , public smoc_chan_adapter<smoc_port_in_if<T>,
      tlm::tlm_nonblocking_peek_if<T> >
{
  template <typename DERIVED, typename CHANTYPE>
  friend class smoc::Detail::ConnectProvider;

  typedef smoc_port_in_if<T> iface_impl_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  smoc_chan_adapter(iface_impl_type &in_if)
    :  smoc_chan_adapter<smoc_port_in_if<T>, tlm::tlm_nonblocking_get_if<T> >(in_if),
       smoc_chan_adapter<smoc_port_in_if<T>, tlm::tlm_nonblocking_peek_if<T> >(in_if) {}
};

#endif /* _INCLUDED_SYSTEMOC_SMOC_CHAN_ADAPTER_HPP */
