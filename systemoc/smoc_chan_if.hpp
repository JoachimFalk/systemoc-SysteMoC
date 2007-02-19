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

#ifndef _INCLUDED_SMOC_CHAN_IF
#define _INCLUDED_SMOC_CHAN_IF

#include "smoc_root_port.hpp"
#include "smoc_event.hpp"
#include "smoc_pggen.hpp"
#include "smoc_storage.hpp"

#include <systemc.h>

#include <list>

template<class S, class T>
class smoc_channel_access {
public:
  typedef S                                              storage_type;
  typedef T                                              return_type;
  typedef smoc_channel_access<storage_type, return_type> this_type;

#ifndef NDEBUG
  virtual void   setLimit(size_t l)                    = 0;
  //virtual size_t getLimit() const                      = 0;
#endif
  virtual return_type operator[](size_t n)             = 0;
  virtual const return_type operator[](size_t n) const = 0;
  virtual bool tokenIsValid(size_t i)                  = 0;
};

template<>
class smoc_channel_access<void, void> {
public:
  typedef void                                           storage_type;
  typedef void                                           return_type;
  typedef smoc_channel_access<storage_type, return_type> this_type;

#ifndef NDEBUG
  virtual void   setLimit(size_t l)                    = 0;
  //virtual size_t getLimit() const                      = 0;
#endif
};

template<>
class smoc_channel_access<const void, const void> {
public:
  typedef const void           storage_type;
  typedef const void           return_type;
  typedef smoc_channel_access<storage_type, return_type> this_type;

#ifndef NDEBUG
  virtual void   setLimit(size_t l)                    = 0;
  //virtual size_t getLimit() const                      = 0;
#endif
};

template<class S, class T>
class smoc_ring_access : public smoc_channel_access<S, T> {
public:
  typedef S                storage_type;
  typedef T                return_type;
  typedef smoc_ring_access<storage_type, return_type> this_type;
private:
#ifndef NDEBUG
  size_t        limit;
#endif
public: // <-- FIXME
  storage_type *storage;
  size_t        storageSize;
  size_t       *offset;
public:
  smoc_ring_access():
#ifndef NDEBUG
      limit(0),
#endif
      storage(NULL), storageSize(0), offset(NULL) {}

#ifndef NDEBUG
  void   setLimit(size_t l) { limit = l; }
//size_t getLimit() const   { return limit; }
#endif

  return_type operator[](size_t n) {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[]" << n << ")" << std::endl;
    assert(n < limit);
    return *offset + n < storageSize
      ? storage[*offset + n]
      : storage[*offset + n - storageSize];
  }
  const return_type operator[](size_t n) const {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[](" << n << ") const" << std::endl;
    assert(n < limit);
    return *offset + n < storageSize
      ? storage[*offset + n]
      : storage[*offset + n - storageSize];
  }

  bool tokenIsValid(size_t n=0){
    // ring_access is used in smoc_fifo -> if any (commited) token is invalid,
    // then it a failure in design
    return true;
  }
};

template <>
class smoc_ring_access<void, void> : public smoc_channel_access<void, void> {
public:
  typedef void                storage_type;
  typedef void                return_type;
  typedef smoc_ring_access<storage_type, return_type> this_type;
private:
#ifndef NDEBUG
  size_t limit;
#endif
public:
  smoc_ring_access()
#ifndef NDEBUG
    : limit(0)
#endif
    {}

#ifndef NDEBUG
  void   setLimit(size_t l) { limit = l; }
//size_t getLimit() const   { return limit; }
#endif
};

template <>
class smoc_ring_access<const void, const void> : public smoc_channel_access<const void, const void> {
public:
  typedef const void              storage_type;
  typedef const void              return_type;
  typedef smoc_ring_access<storage_type, return_type> this_type;
private:
#ifndef NDEBUG
  size_t limit;
#endif
public:
  smoc_ring_access()
#ifndef NDEBUG
    : limit(0)
#endif
    {}

#ifndef NDEBUG
  void   setLimit(size_t l) { limit = l; }
//size_t getLimit() const   { return limit; }
#endif
};

template <typename T, template <typename, typename> class R, class PARAM_TYPE>
class smoc_port_in_base;
template <typename T, 
          template <typename, typename> class R, 
          class PARAM_TYPE, 
          template <typename> class STORAGE_TYPE>
class smoc_port_out_base;

class smoc_chan_in_base_if {
public:
  template <typename T, template <typename, typename> class R, class PARAM_TYPE> friend class smoc_port_in_base;
private:
  smoc_port_list portsIn;
protected:
  // constructor
  smoc_chan_in_base_if() {}

  void addPort(smoc_root_port_in  *portIn)
    { portsIn.push_front(portIn); }
public:
  const smoc_port_list &getInputPorts()  const
    { return portsIn;  }
};

class smoc_chan_out_base_if {
public:
  template <typename T, template <typename, typename> class R, class PARAM_TYPE, template <typename> class STORAGE_TYPE> friend class smoc_port_out_base;
private:
  smoc_port_list portsOut;
protected:
  // constructor
  smoc_chan_out_base_if() {}

  void addPort(smoc_root_port_out *portOut)
    { portsOut.push_front(portOut); }
public:
  const smoc_port_list &getOutputPorts() const
    { return portsOut; }
};

template <typename T, template <typename, typename> class R>
class smoc_chan_in_if
  : virtual public sc_interface,
    virtual public smoc_chan_in_base_if {
public:
  // typedefs
  typedef smoc_chan_in_if<T,R>      this_type;
  typedef T          data_type;
  typedef R<
    typename smoc_storage_in<T>::storage_type,
    typename smoc_storage_in<T>::return_type>   access_type;
  typedef access_type                           access_in_type;
  
  virtual size_t numAvailable() const = 0;
//smoc_event &dataAvailableEvent(size_t n) { return write_event; }
  virtual smoc_event &dataAvailableEvent(size_t n) = 0;
  virtual access_in_type * accessSetupIn() = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void   commitRead(size_t consume, const smoc_ref_event_p &) = 0;
#else
  virtual void   commitRead(size_t consume) = 0;
#endif
protected:
//smoc_event write_event;
  
  // constructor
  smoc_chan_in_if() {}
//  // write_event start unnotified
//  : write_event(false) {}
private:
  // disabled
  const sc_event& default_event() const = 0;
  // disabled
  smoc_chan_in_if( const this_type& );
  this_type &operator = ( const this_type & );
};

template <typename T,                                    //data type
          template <typename, typename> class R,         //ring access type
          template <typename> class S = smoc_storage_out //smoc_storage
          >
class smoc_chan_out_if
  : virtual public sc_interface,
    virtual public smoc_chan_out_base_if {
public:
  // typedefs
            typedef smoc_chan_out_if<T,R,S>      this_type;
  typedef T          data_type;
  typedef R<
    typename S<T>::storage_type,
    typename S<T>::return_type>  access_type;
  typedef access_type                           access_out_type;
  
  virtual size_t      numFree() const = 0;
//smoc_event    &spaceAvailableEvent(size_t n) { return read_event; }
  virtual smoc_event &spaceAvailableEvent(size_t n) = 0;
  virtual access_out_type * accessSetupOut() = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void commitWrite(size_t produce, const smoc_ref_event_p &) = 0;
#else
  virtual void commitWrite(size_t produce) = 0;
#endif
protected:
//smoc_event read_event;
  
  // constructor
  smoc_chan_out_if() {}
//  // read_event start unnotified
//  : read_event(false) {}
private:
  // disabled
  const sc_event& default_event() const = 0;
  // disabled
  smoc_chan_out_if( const this_type& );
  this_type& operator = ( const this_type & );
};

class smoc_root_chan
  : public sc_prim_channel,
    virtual public smoc_chan_in_base_if,
    virtual public smoc_chan_out_base_if {
public:
  // typedefs
  typedef smoc_root_chan              this_type;
  
  friend class smoc_graph;
protected:
  std::string myName; // patched in finalise
public:
  const char *name() const { return myName.c_str(); }
  virtual void channelContents(smoc_modes::PGWriter &pgw)   const = 0;
  virtual void channelAttributes(smoc_modes::PGWriter &pgw) const = 0;
  
  virtual void finalise();
protected:
  // constructor
  smoc_root_chan(const char *name)
    : sc_prim_channel(name) {}

  virtual void assemble(smoc_modes::PGWriter &pgw) const = 0;
};

class smoc_nonconflicting_chan
  : public smoc_root_chan {
public:
  // typedefs
  typedef smoc_nonconflicting_chan this_type;
protected:
  // constructor
  smoc_nonconflicting_chan(const char *name)
    : smoc_root_chan(name) {}

  void assemble(smoc_modes::PGWriter &pgw) const;

  virtual void finalise();
};

class smoc_multicast_chan
  : public smoc_root_chan {
public:
  // typedefs
  typedef smoc_multicast_chan this_type;
protected:
  // constructor
  smoc_multicast_chan(const char *name)
    : smoc_root_chan(name) {}

  void assemble(smoc_modes::PGWriter &pgw) const;

  virtual void finalise();
};

extern const sc_event& smoc_default_event_abort();

template <typename T_chan_kind, 
          typename T_data_type, 
          template <typename, typename> class R_IN, //ring access in
          template <typename, typename> class R_OUT,//ring access out
          template <typename> class S = smoc_storage_out
          >
class smoc_chan_if
  : public smoc_chan_in_if<T_data_type, R_IN>,
    public smoc_chan_out_if<T_data_type, R_OUT, S>,
    public T_chan_kind {
public:
  // typedefs
  typedef smoc_chan_if<T_chan_kind,T_data_type,R_IN,R_OUT,S> this_type;
  typedef T_data_type                             data_type;
  typedef T_chan_kind                             chan_kind;

protected:
  // constructor
  smoc_chan_if(const typename chan_kind::chan_init &i)
    : chan_kind(i) {}
private:
  // disabled
  const sc_event& default_event() const { return smoc_default_event_abort(); }

  virtual void reset(){};
};

typedef std::list<smoc_root_chan *> smoc_chan_list;

#include "smoc_port.hpp"

#endif // _INCLUDED_SMOC_CHAN_IF
