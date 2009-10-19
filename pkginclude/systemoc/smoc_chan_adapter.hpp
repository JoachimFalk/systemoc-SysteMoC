// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SMOC_CHAN_ADAPTER_HPP
#define _INCLUDED_SMOC_CHAN_ADAPTER_HPP

#include <tlm.h>

#include <boost/type_traits/is_base_of.hpp>

#include <systemoc/smoc_config.h>

#include "detail/smoc_chan_if.hpp"

namespace tlm {

/**
 * Specializations of tlm interfaces for void 
 * (can't use (const) reference to void as parameter!)
 */

// get interfaces
template<>
class tlm_blocking_get_if<void> : public virtual sc_core::sc_interface {
public:
  virtual void get(tlm_tag<void> * = NULL) = 0; // can't use "void&"
};

template<>
class tlm_nonblocking_get_if<void> : public virtual sc_core::sc_interface {
public:
  virtual bool nb_get(tlm_tag<void> * = NULL) = 0; // can't use "void&"
  virtual bool nb_can_get(tlm_tag<void> * = NULL) const = 0;
  virtual const sc_core::sc_event &ok_to_get(tlm_tag<void> * = NULL) const = 0;
};

// peek interfaces
template<>
class tlm_blocking_peek_if<void> : public virtual sc_core::sc_interface {
public:
  virtual void peek(tlm_tag<void> * = NULL) const = 0;
};

template<>
class tlm_nonblocking_peek_if<void> : public virtual sc_core::sc_interface {
public:
  virtual bool nb_peek(tlm_tag<void> * = NULL) const = 0;
  virtual bool nb_can_peek(tlm_tag<void> * = NULL) const = 0;
  virtual const sc_core::sc_event &ok_to_peek(tlm_tag<void> * = NULL) const = 0;
};

// put interfaces
template<>
class tlm_blocking_put_if<void> : public virtual sc_core::sc_interface {
public:
  virtual void put(tlm_tag<void> * = NULL) = 0; // can't use "const void&"
};

template<>
class tlm_nonblocking_put_if<void> : public virtual sc_core::sc_interface {
public:
  virtual bool nb_put(tlm_tag<void> * = NULL) = 0; // can't use "const void&"
  virtual bool nb_can_put(tlm_tag<void> * = NULL) const = 0;
  virtual const sc_core::sc_event &ok_to_put(tlm_tag<void> * = NULL) const = 0;
};

} // namespace tlm

namespace SysteMoC { namespace Detail {

  /// select type A or B based on predicate P
  template<bool P, class A, class B>
  struct Select;

  /// specialization: select type A
  template<class A,class B>
  struct Select<true,A,B>
    { typedef A result_type; };

  /// specialization: select type B
  template<class A,class B>
  struct Select<false,A,B>
    { typedef B result_type; };

  class ChanAdapterBase
  : public virtual sc_core::sc_interface {
  private:
    smoc_port_base_if &iface;
  protected:
    ChanAdapterBase(smoc_port_base_if &iface)
      : iface(iface) {}
  public:
    smoc_port_base_if       &getIface()
      { return iface; }
    smoc_port_base_if const &getIface() const
      { return iface; }
  };

  template<class IFaceImpl>
  class ChanAdapterMid;

  template<class T, template<class> class R>
  class ChanAdapterMid<smoc_port_in_if<T,R> >
  : public ChanAdapterBase {
    typedef ChanAdapterMid<smoc_port_in_if<T,R> > this_type;
    typedef ChanAdapterBase                       base_type;
  public:
    /// typedefs
    typedef smoc_port_in_if<T,R> iface_impl_type;
  protected:
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
    void register_port(sc_port_base &p, const char *if_ty)
      { getIface().register_port(p, typeid(iface_impl_type).name()); }
  };

  template<class T, template<class> class R, template<class> class S>
  class ChanAdapterMid<smoc_port_out_if<T,R,S> >
  : public ChanAdapterBase {
    typedef ChanAdapterMid<smoc_port_out_if<T,R,S> > this_type;
    typedef ChanAdapterBase                          base_type;
  public:
    /// typedefs
    typedef smoc_port_out_if<T,R,S> iface_impl_type;
  protected:
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
    void register_port(sc_port_base &p, const char *if_ty)
      { getIface().register_port(p, typeid(iface_impl_type).name()); }
  };

} } // namespace SysteMoC::Detail


/**
 * specialize this class to obtain an adapter from IFace to IFaceImpl
 * - derive class implements IFace and Detail::ChanAdapterMid
 * - provide an instance of IFaceImpl (it's an adapter, it needs not
 *   implement IFaceImpl)
 * - set isAdapter to true
 */
template<class IFaceImpl, class IFace>
class smoc_chan_adapter {
public:
  /// typedefs
  typedef IFaceImpl iface_impl_type;
  typedef IFace     iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = false;
};

/**
 * adapter specialization for blocking tlm get -> smoc channel read
 */
template<class T, template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<T,R>,
    tlm::tlm_blocking_get_if<T> >
: public tlm::tlm_blocking_get_if<T>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_in_if<T,R> > {
  typedef smoc_chan_adapter<smoc_port_in_if<T,R>,  tlm::tlm_blocking_get_if<T> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_in_if<T,R> >(in_if) {}

  /// see tlm::tlm_blocking_get_if<T>
  T get(tlm::tlm_tag<T> * = NULL) {
    
    wait(this->dataAvailable);
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    const T &t = (*ca)[0];
    
#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    this->getIface().commitRead(1u,lat);
#else
    this->getIface().commitRead(1u);
#endif
    
    return t;
  }
};

/**
 * adapter specialization for blocking tlm get -> smoc channel read
 */
template<template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<void,R>,
    tlm::tlm_blocking_get_if<void> >
: public tlm::tlm_blocking_get_if<void>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_in_if<void,R> > {
  typedef smoc_chan_adapter<smoc_port_in_if<void,R>,  tlm::tlm_blocking_get_if<void> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_in_if<void,R> >(in_if) {}

  /// see tlm::tlm_blocking_get_if<void>
  void get(tlm::tlm_tag<void> * = NULL) {
    wait(this->dataAvailable);
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    this->getIface().commitRead(1u,lat);
#else
    this->getIface().commitRead(1u);
#endif
  }
};

/**
 * adapter specialization for nonblocking tlm get -> smoc channel read
 */
template<class T, template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<T,R>,
    tlm::tlm_nonblocking_get_if<T> >
: public tlm::tlm_nonblocking_get_if<T>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_in_if<T,R> > {
  typedef smoc_chan_adapter<smoc_port_in_if<T,R>,  tlm::tlm_nonblocking_get_if<T> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_in_if<T,R> >(in_if),
      scev(this->dataAvailable) {}

  /// see tlm_nonblocking_get_if<T>
  bool nb_get(T &t) {
    if (!this->dataAvailable)
      return false;
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    t = (*ca)[0];
    
#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    this->getIface().commitRead(1u,lat);
#else
    this->getIface().commitRead(1u);
#endif
    
    return true;
  }
  
  /// see tlm_nonblocking_get_if<T>
  bool nb_can_get(tlm::tlm_tag<T> * = NULL) const
    { return this->dataAvailable; }

  /// see tlm_nonblocking_get_if<T>
  const sc_core::sc_event& ok_to_get(tlm::tlm_tag<T> * = NULL) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for nonblocking tlm get -> smoc channel read
 */
template<template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<void,R>,
    tlm::tlm_nonblocking_get_if<void> >
: public tlm::tlm_nonblocking_get_if<void>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_in_if<void,R> > {
  typedef smoc_chan_adapter<smoc_port_in_if<void,R>,  tlm::tlm_nonblocking_get_if<void> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_in_if<void,R> >(in_if),
      scev(this->dataAvailable) {}

  /// see tlm_nonblocking_get_if<void>
  bool nb_get(tlm::tlm_tag<void> * = NULL) {
    if (!this->dataAvailable)
      return false;
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    this->getIface().commitRead(1u,lat);
#else
    this->getIface().commitRead(1u);
#endif
    
    return true;
  }

  /// see tlm_nonblocking_get_if<void>
  bool nb_can_get(tlm::tlm_tag<void> * = NULL) const
    { return this->dataAvailable; }

  /// see tlm_nonblocking_get_if<void>
  const sc_core::sc_event& ok_to_get(tlm::tlm_tag<void> * = NULL) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for blocking tlm peek -> smoc channel read
 */
template<class T, template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<T,R>,
    tlm::tlm_blocking_peek_if<T> >
: public tlm::tlm_blocking_peek_if<T>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_in_if<T,R> > {
  typedef smoc_chan_adapter<smoc_port_in_if<T,R>,  tlm::tlm_blocking_peek_if<T> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_in_if<T,R> >(in_if) {}

  T peek(tlm::tlm_tag<T> *t = NULL) const {
    wait(this->dataAvailable);
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
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
template<template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<void,R>,
    tlm::tlm_blocking_peek_if<void> >
: public tlm::tlm_blocking_peek_if<void>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_in_if<void,R> > {
  typedef smoc_chan_adapter<smoc_port_in_if<void,R>,  tlm::tlm_blocking_peek_if<void> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_in_if<void,R> >(in_if) {}

  void peek(tlm::tlm_tag<void> *t = NULL) const {
    wait(this->dataAvailable);
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
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
template<class T, template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<T,R>,
    tlm::tlm_nonblocking_peek_if<T> >
: public tlm::tlm_nonblocking_peek_if<T>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_in_if<T,R> > {
  typedef smoc_chan_adapter<smoc_port_in_if<T,R>, tlm::tlm_nonblocking_peek_if<T> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_in_if<T,R> >(in_if),
      scev(this->dataAvailable) {}

  /// see tlm_nonblocking_peek_if<T>
  bool nb_peek(T &t) {
    if (!this->dataAvailable)
      return false;
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().peekChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    t = (*ca)[0];
    
    return true;
  }

  /// see tlm_nonblocking_peek_if<T>
  bool nb_can_peek(tlm::tlm_tag<T> * = NULL) const
    { return this->dataAvailable; }

  /// see tlm_nonblocking_peek_if<T>
  const sc_core::sc_event& ok_to_peek(tlm::tlm_tag<T> * = NULL) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for nonblocking tlm peek -> smoc channel read
 */
template<template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<void,R>,
    tlm::tlm_nonblocking_peek_if<void> >
: public tlm::tlm_nonblocking_peek_if<void>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_in_if<void,R> > {
  typedef smoc_chan_adapter<smoc_port_in_if<void,R>, tlm::tlm_nonblocking_peek_if<void> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_in_if<void,R> >(in_if),
      scev(this->dataAvailable) {}

  /// see tlm_nonblocking_peek_if<void>
  bool nb_peek(tlm::tlm_tag<void> * = NULL) {
    if (!this->dataAvailable)
      return false;
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().peekChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
    return true;
  }

  /// see tlm_nonblocking_peek_if<void>
  bool nb_can_peek(tlm::tlm_tag<void> * = NULL) const
    { return this->dataAvailable; }

  /// see tlm_nonblocking_peek_if<void>
  const sc_core::sc_event& ok_to_peek(tlm::tlm_tag<void> * = NULL) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for blocking tlm put -> smoc channel write
 */
template<class T, template<class> class R, template<class> class S>
class smoc_chan_adapter<
    smoc_port_out_if<T,R,S>,
    tlm::tlm_blocking_put_if<T> >
: public tlm::tlm_blocking_put_if<T>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_out_if<T,R> > {
  typedef smoc_chan_adapter<smoc_port_out_if<T,R,S>,  tlm::tlm_blocking_put_if<T> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &out_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_out_if<T,R,S> >(out_if) {}

  /// see tlm::tlm_blocking_put_if<T>
  void put(const T &t) {
    wait(this->spaceAvailable);
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    (*ca)[0] = t;
    
#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    this->getIface().commitWrite(1u,lat);
#else
    this->getIface().commitWrite(1u);
#endif
  }
};

/**
 * adapter specialization for blocking tlm put -> smoc channel write
 */
template<template<class> class R, template<class> class S>
class smoc_chan_adapter<
    smoc_port_out_if<void,R,S>,
    tlm::tlm_blocking_put_if<void> >
: public tlm::tlm_blocking_put_if<void>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_out_if<void,R,S> > {
  typedef smoc_chan_adapter<smoc_port_out_if<void,R,S>,  tlm::tlm_blocking_put_if<void> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &out_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_out_if<void,R,S> >(out_if) {}

  /// see tlm::tlm_blocking_put_if<void>
  void put(tlm::tlm_tag<void> * = NULL) {
    wait(this->spaceAvailable);
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    this->getIface().commitWrite(1u,lat);
#else
    this->getIface().commitWrite(1u);
#endif
  }
};

/**
 * adapter specialization for nonblocking tlm put -> smoc channel write
 */
template<class T, template<class> class R, template<class> class S>
class smoc_chan_adapter<
    smoc_port_out_if<T,R,S>,
    tlm::tlm_nonblocking_put_if<T> >
: public tlm::tlm_nonblocking_put_if<T>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_out_if<T,R,S> > {
  typedef smoc_chan_adapter<smoc_port_out_if<T,R,S>, tlm::tlm_nonblocking_put_if<T> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &out_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_out_if<T,R,S> >(out_if),
      scev(this->spaceAvailable) {}

  /// see tlm::tlm_nonblocking_put_if<T>
  bool nb_put(const T &t) {
    if (!this->spaceAvailable)
      return false;
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    (*ca)[0] = t;
    
#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    this->getIface().commitWrite(1u,lat);
#else
    this->getIface().commitWrite(1u);
#endif
    
    return true;
  }

  /// see tlm::tlm_nonblocking_put_if<T>
  bool nb_can_put(tlm::tlm_tag<T> * = NULL) const
    { return this->spaceAvailable; }

  /// see tlm::tlm_nonblocking_put_if<T>
  const sc_core::sc_event &ok_to_put(tlm::tlm_tag<T> * = NULL) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for nonblocking tlm put -> smoc channel write
 */
template<template<class> class R, template<class> class S>
class smoc_chan_adapter<
    smoc_port_out_if<void,R,S>,
    tlm::tlm_nonblocking_put_if<void> >
: public tlm::tlm_nonblocking_put_if<void>,
  public SysteMoC::Detail::ChanAdapterMid
    <smoc_port_out_if<void,R,S> > {
  typedef smoc_chan_adapter<smoc_port_out_if<void,R,S>, tlm::tlm_nonblocking_put_if<void> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;
protected:
  CoSupport::SystemC::SCEventWrapper scev;
public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(typename this_type::iface_impl_type &out_if)
    : SysteMoC::Detail::ChanAdapterMid<smoc_port_out_if<void,R,S> >(out_if),
      scev(this->spaceAvailable) {}

  /// see tlm::tlm_nonblocking_put_if<void>
  bool nb_put(tlm::tlm_tag<void> * = NULL) {
    if (!this->spaceAvailable)
      return false;
    
    typename this_type::iface_impl_type::access_type *ca =
      this->getIface().getChannelAccess();
    
#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    
#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    this->getIface().commitWrite(1u,lat);
#else
    this->getIface().commitWrite(1u);
#endif
    
    return true;
  }

  /// see tlm::tlm_nonblocking_put_if<void>
  bool nb_can_put(tlm::tlm_tag<void> * = NULL) const
    { return this->spaceAvailable; }

  /// see tlm::tlm_nonblocking_put_if<void>
  const sc_core::sc_event &ok_to_put(tlm::tlm_tag<void> * = NULL) const
    { return scev.getSCEvent(); }
};

/**
 * adapter specialization for nonblocking tlm get and peek -> smoc channel read
 */
template<class T, template<class> class R>
class smoc_chan_adapter<
    smoc_port_in_if<T,R>,
    tlm::tlm_nonblocking_get_peek_if<T> >
: public smoc_chan_adapter<smoc_port_in_if<T,R>,
    tlm::tlm_nonblocking_get_if<T> >,
  public smoc_chan_adapter<smoc_port_in_if<T,R>,
    tlm::tlm_nonblocking_peek_if<T> > {
  typedef smoc_chan_adapter<smoc_port_in_if<T,R>,  tlm::tlm_nonblocking_get_peek_if<T> > this_type;
public:
  /// flag if this class is a specialization
  static const bool isAdapter = true;

  smoc_chan_adapter(typename this_type::iface_impl_type &in_if)
    :  smoc_chan_adapter<smoc_port_in_if<T,R>, tlm::tlm_nonblocking_get_if<T> >(in_if),
       smoc_chan_adapter<smoc_port_in_if<T,R>, tlm::tlm_nonblocking_peek_if<T> >(in_if) {}
};

#endif // _INCLUDED_SMOC_CHAN_ADAPTER_HPP
