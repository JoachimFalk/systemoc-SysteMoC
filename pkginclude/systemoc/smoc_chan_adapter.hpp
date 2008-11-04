// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
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

#include <systemoc/smoc_chan_if.hpp>
#include <tlm.h>

#include <boost/type_traits/is_base_of.hpp>

namespace tlm {

/**
 * Specializations of tlm interfaces for void 
 * (can't use (const) reference to void as parameter!)
 */
template<>
class tlm_blocking_get_if<void> : public virtual sc_core::sc_interface {
public:
  virtual void get(tlm_tag<void>* = 0) = 0; // can't use "void&"
};

template<>
class tlm_blocking_put_if<void> : public virtual sc_core::sc_interface {
public:
  virtual void put(tlm_tag<void>* = 0) = 0; // can't use "const void&"
};

template<>
class tlm_nonblocking_get_if<void> : public virtual sc_core::sc_interface {
public:
  virtual bool nb_get(tlm_tag<void>* = 0) = 0; // can't use "void&"
  virtual bool nb_can_get(tlm_tag<void>* = 0) const = 0;
  virtual const sc_core::sc_event &ok_to_get(tlm_tag<void>* = 0) const = 0;
};

template<>
class tlm_nonblocking_put_if<void> : public virtual sc_core::sc_interface {
public:
  virtual bool nb_put(tlm_tag<void>* = 0) = 0; // can't use "const void&"
  virtual bool nb_can_put(tlm_tag<void>* = 0) const = 0;
  virtual const sc_core::sc_event &ok_to_put(tlm_tag<void>* = 0) const = 0;
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

}} // namespace SysteMoC::Detail

/**
 * specialize this class to obtain an adapter from IFace to IFaceImpl
 * - derived class implements IFace
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
    smoc_chan_in_if<T,R>,
    tlm::tlm_blocking_get_if<T>
  > :
  public virtual tlm::tlm_blocking_get_if<T>
{
public:
  /// typedefs
  typedef smoc_chan_in_if<T,R>        iface_impl_type;
  typedef tlm::tlm_blocking_get_if<T> iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type& in_if) :
    in_if(in_if),
    dataAvailable(in_if.dataAvailableEvent(1))
  {}

  /// see tlm::tlm_blocking_get_if<T>
  T get(tlm::tlm_tag<T>* = 0) {
    
    wait(dataAvailable);
    
    typename iface_impl_type::access_type* ca =
      in_if.getChannelAccess();

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    const T& t = (*ca)[0];

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    in_if.commitRead(1u,lat);
#else
    in_if.commitRead(1u);
#endif

    return t;
  }

private:
  iface_impl_type& in_if;
  smoc_event& dataAvailable;
};

/**
 * adapter specialization for blocking tlm get -> smoc channel read
 */
template<template<class> class R>
class smoc_chan_adapter<
    smoc_chan_in_if<void,R>,
    tlm::tlm_blocking_get_if<void>
  > :
  public virtual tlm::tlm_blocking_get_if<void>
{
public:
  /// typedefs
  typedef void T;
  typedef smoc_chan_in_if<T,R>        iface_impl_type;
  typedef tlm::tlm_blocking_get_if<T> iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type& in_if) :
    in_if(in_if),
    dataAvailable(in_if.dataAvailableEvent(1))
  {}

  /// see tlm::tlm_blocking_get_if<T>
  void get(tlm::tlm_tag<T>* = 0) {
    
    wait(dataAvailable);
    
    typename iface_impl_type::access_type* ca =
      in_if.getChannelAccess();

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    in_if.commitRead(1u,lat);
#else
    in_if.commitRead(1u);
#endif
  }

private:
  iface_impl_type& in_if;
  smoc_event& dataAvailable;
};


/**
 * adapter specialization for blocking tlm put -> smoc channel write
 */
template<class T, template<class> class R, template<class> class S>
class smoc_chan_adapter<
    smoc_chan_out_if<T,R,S>,
    tlm::tlm_blocking_put_if<T>
  > :
  public virtual tlm::tlm_blocking_put_if<T>
{
public:
  /// typedefs
  typedef smoc_chan_out_if<T,R,S>     iface_impl_type;
  typedef tlm::tlm_blocking_put_if<T> iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type& out_if) :
    out_if(out_if),
    spaceAvailable(out_if.spaceAvailableEvent(1))
  {}

  /// see tlm::tlm_blocking_put_if<T>
  void put(const T& t) {

    wait(spaceAvailable);
    
    typename iface_impl_type::access_type* ca =
      out_if.getChannelAccess();

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    (*ca)[0] = t;

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    out_if.commitWrite(1u,lat);
#else
    out_if.commitWrite(1u);
#endif
  }

private:
  iface_impl_type& out_if;
  smoc_event& spaceAvailable;
};

/**
 * adapter specialization for blocking tlm put -> smoc channel write
 */
template<template<class> class R, template<class> class S>
class smoc_chan_adapter<
    smoc_chan_out_if<void,R,S>,
    tlm::tlm_blocking_put_if<void>
  > :
  public virtual tlm::tlm_blocking_put_if<void>
{
public:
  /// typedefs
  typedef void T;
  typedef smoc_chan_out_if<T,R,S>     iface_impl_type;
  typedef tlm::tlm_blocking_put_if<T> iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type& out_if) :
    out_if(out_if),
    spaceAvailable(out_if.spaceAvailableEvent(1))
  {}

  /// see tlm::tlm_blocking_put_if<T>
  void put(tlm::tlm_tag<void>* = 0) {

    wait(spaceAvailable);
    
    typename iface_impl_type::access_type* ca =
      out_if.getChannelAccess();

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    out_if.commitWrite(1u,lat);
#else
    out_if.commitWrite(1u);
#endif
  }

private:
  iface_impl_type& out_if;
  smoc_event& spaceAvailable;
};

/**
 * adapter specialization for nonblocking tlm get -> smoc channel read
 */
template<class T, template<class> class R>
class smoc_chan_adapter<
    smoc_chan_in_if<T,R>,
    tlm::tlm_nonblocking_get_if<T>
  > :
  public virtual tlm::tlm_nonblocking_get_if<T>
{
public:
  /// typedefs
  typedef smoc_chan_in_if<T,R>           iface_impl_type;
  typedef tlm::tlm_nonblocking_get_if<T> iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type& in_if) :
    in_if(in_if),
    dataAvailable(in_if.dataAvailableEvent(1)),
    scev(dataAvailable)
  {}

  /// see tlm_nonblocking_get_if<T>
  bool nb_get(T& t) {

    if(!dataAvailable) return false;

    typename iface_impl_type::access_type* ca =
      in_if.getChannelAccess();

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    t = (*ca)[0];

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    in_if.commitRead(1u,lat);
#else
    in_if.commitRead(1u);
#endif

    return true;
  }
  
  /// see tlm_nonblocking_get_if<T>
  bool nb_can_get(tlm::tlm_tag<T>* = 0) const
  { return dataAvailable; }

  /// see tlm_nonblocking_get_if<T>
  const sc_core::sc_event& ok_to_get(tlm::tlm_tag<T>* = 0) const
  { return scev.getSCEvent(); }

protected:
  /// @brief See sc_interface
  void register_port(sc_port_base& p, const char* if_ty)
  { in_if.register_port(p, typeid(iface_impl_type).name()); }

private:
  iface_impl_type& in_if;
  smoc_event& dataAvailable;
  CoSupport::SystemC::SCEventWrapper scev;
};

/**
 * adapter specialization for nonblocking tlm get -> smoc channel read
 */
template<template<class> class R>
class smoc_chan_adapter<
    smoc_chan_in_if<void,R>,
    tlm::tlm_nonblocking_get_if<void>
  > :
  public virtual tlm::tlm_nonblocking_get_if<void>
{
public:
  /// typedefs
  typedef void T;
  typedef smoc_chan_in_if<T,R>           iface_impl_type;
  typedef tlm::tlm_nonblocking_get_if<T> iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs read channel access
  smoc_chan_adapter(iface_impl_type& in_if) :
    in_if(in_if),
    dataAvailable(in_if.dataAvailableEvent(1)),
    scev(dataAvailable)
  {}

  /// see tlm_nonblocking_get_if<T>
  bool nb_get(tlm::tlm_tag<T>* = 0) {

    if(!dataAvailable) return false;

    typename iface_impl_type::access_type* ca =
      in_if.getChannelAccess();

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    in_if.commitRead(1u,lat);
#else
    in_if.commitRead(1u);
#endif

    return true;
  }
  
  /// see tlm_nonblocking_get_if<T>
  bool nb_can_get(tlm::tlm_tag<T>* = 0) const
  { return dataAvailable; }

  /// see tlm_nonblocking_get_if<T>
  const sc_core::sc_event& ok_to_get(tlm::tlm_tag<T>* = 0) const
  { return scev.getSCEvent(); }

protected:
  /// @brief See sc_interface
  void register_port(sc_port_base& p, const char* if_ty)
  { in_if.register_port(p, typeid(iface_impl_type).name()); }

private:
  iface_impl_type& in_if;
  smoc_event& dataAvailable;
  CoSupport::SystemC::SCEventWrapper scev;
};

/**
 * adapter specialization for nonblocking tlm put -> smoc channel write
 */
template<class T, template<class> class R, template<class> class S>
class smoc_chan_adapter<
    smoc_chan_out_if<T,R,S>,
    tlm::tlm_nonblocking_put_if<T>
  > :
  public virtual tlm::tlm_nonblocking_put_if<T>
{
public:
  /// typedefs
  typedef smoc_chan_out_if<T,R,S>         iface_impl_type;
  typedef tlm::tlm_nonblocking_put_if<T>  iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type& out_if) :
    out_if(out_if),
    spaceAvailable(out_if.spaceAvailableEvent(1)),
    scev(spaceAvailable)
  {}

  /// see tlm::tlm_nonblocking_put_if<T>
  bool nb_put(const T& t) {

    if(!spaceAvailable) return false;
    
    typename iface_impl_type::access_type* ca =
      out_if.getChannelAccess();

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif
    (*ca)[0] = t;

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    out_if.commitWrite(1u,lat);
#else
    out_if.commitWrite(1u);
#endif

    return true;
  }

  /// see tlm::tlm_nonblocking_put_if<T>
  bool nb_can_put(tlm::tlm_tag<T>* = 0) const
  { return spaceAvailable; }

  /// see tlm::tlm_nonblocking_put_if<T>
  const sc_core::sc_event& ok_to_put(tlm::tlm_tag<T>* = 0) const
  { return scev.getSCEvent(); }

protected:
  /// @brief See sc_interface
  void register_port(sc_port_base& p, const char* if_ty)
  { out_if.register_port(p, typeid(iface_impl_type).name()); }

private:
  iface_impl_type& out_if;
  smoc_event& spaceAvailable;
  CoSupport::SystemC::SCEventWrapper scev;
};

/**
 * adapter specialization for nonblocking tlm put -> smoc channel write
 */
template<template<class> class R, template<class> class S>
class smoc_chan_adapter<
    smoc_chan_out_if<void,R,S>,
    tlm::tlm_nonblocking_put_if<void>
  > :
  public virtual tlm::tlm_nonblocking_put_if<void>
{
public:
  /// typedefs
  typedef void T;
  typedef smoc_chan_out_if<T,R,S>        iface_impl_type;
  typedef tlm::tlm_nonblocking_put_if<T> iface_type;

  /// flag if this class is a specialization
  static const bool isAdapter = true;

public:
  /// constructor
  /// - stores reference to wrapped interface
  /// - needs write channel access
  smoc_chan_adapter(iface_impl_type& out_if) :
    out_if(out_if),
    spaceAvailable(out_if.spaceAvailableEvent(1)),
    scev(spaceAvailable)
  {}

  /// see tlm::tlm_nonblocking_put_if<T>
  bool nb_put(tlm::tlm_tag<T>* = 0) {

    if(!spaceAvailable) return false;
    
    typename iface_impl_type::access_type* ca =
      out_if.getChannelAccess();

#if defined(SYSTEMOC_ENABLE_DEBUG)
    // why must we set the limit? 
    ca->setLimit(1); 
#endif

#ifdef SYSTEMOC_ENABLE_VPC
    // start notified
    smoc_ref_event_p lat = new smoc_ref_event(true);
    out_if.commitWrite(1u,lat);
#else
    out_if.commitWrite(1u);
#endif

    return true;
  }

  /// see tlm::tlm_nonblocking_put_if<T>
  bool nb_can_put(tlm::tlm_tag<T>* = 0) const
  { return spaceAvailable; }

  /// see tlm::tlm_nonblocking_put_if<T>
  const sc_core::sc_event& ok_to_put(tlm::tlm_tag<T>* = 0) const
  { return scev.getSCEvent(); }

protected:
  /// @brief See sc_interface
  void register_port(sc_port_base& p, const char* if_ty)
  { out_if.register_port(p, typeid(iface_impl_type).name()); }

private:
  iface_impl_type& out_if;
  smoc_event& spaceAvailable;
  CoSupport::SystemC::SCEventWrapper scev;
};


#endif // _INCLUDED_SMOC_CHAN_ADAPTER_HPP
