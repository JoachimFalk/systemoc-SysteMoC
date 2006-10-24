// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _INCLUDED_SMOC_CHAN_IF
#define _INCLUDED_SMOC_CHAN_IF

#include <smoc_root_port.hpp>
#include <smoc_event.hpp>
#include <smoc_pggen.hpp>
#include <smoc_storage.hpp>

#include <systemc.h>

#include <list>

template<class S, class T>
class smoc_ring_access {
public:
  typedef S					      storage_type;
  typedef T					      return_type;
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
};

template <>
class smoc_ring_access<void, void> {
public:
  typedef void					      storage_type;
  typedef void					      return_type;
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
class smoc_ring_access<const void, const void> {
public:
  typedef const void				      storage_type;
  typedef const void				      return_type;
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

template <typename T>
class smoc_port_in;
template <typename T>
class smoc_port_out;

class smoc_chan_in_base_if {
public:
  template <typename T> friend class smoc_port_in;
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
  template <typename T> friend class smoc_port_out;
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

template <typename T>
class smoc_chan_in_if
  : virtual public sc_interface,
    virtual public smoc_chan_in_base_if {
public:
  // typedefs
  typedef T						    data_type;
  typedef smoc_chan_in_if<data_type>			    this_type;
  typedef typename smoc_storage_in<data_type>::storage_type storage_type;
  typedef typename smoc_storage_in<data_type>::return_type  return_type;
  typedef smoc_ring_access<storage_type, return_type>	    ring_in_type;
  
  bool is_v1_in_port;
  
  virtual size_t committedOutCount() const = 0;
//smoc_event &blockEventOut(size_t n) { return write_event; }
  virtual smoc_event &blockEventOut(size_t n) = 0;
  virtual void   ringSetupIn(ring_in_type &r) = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void   commExecIn(size_t consume, const smoc_ref_event_p &) = 0;
#else
  virtual void   commExecIn(size_t consume) = 0;
#endif
  virtual bool   portOutIsV1() const = 0;
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

template <typename T>
class smoc_chan_out_if
  : virtual public sc_interface,
    virtual public smoc_chan_out_base_if {
public:
  // typedefs
  typedef T						     data_type;
  typedef smoc_chan_out_if<T>				     this_type;
  typedef typename smoc_storage_out<data_type>::storage_type storage_type;
  typedef typename smoc_storage_out<data_type>::return_type  return_type;
  typedef smoc_ring_access<storage_type, return_type>	     ring_out_type;
  
  bool is_v1_out_port;
  
  virtual size_t      committedInCount() const = 0;
//smoc_event    &blockEventIn(size_t n) { return read_event; }
  virtual smoc_event &blockEventIn(size_t n) = 0;
  virtual void        ringSetupOut(ring_out_type &r) = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void        commExecOut(size_t produce, const smoc_ref_event_p &) = 0;
#else
  virtual void        commExecOut(size_t produce) = 0;
#endif
  virtual bool        portInIsV1() const = 0;
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
  smoc_root_port *portIn;
  smoc_root_port *portOut;
public:
  virtual void finalise();
protected:
  // constructor
  smoc_nonconflicting_chan(const char *name)
    : smoc_root_chan(name), portIn(NULL), portOut(NULL) {}

  void assemble(smoc_modes::PGWriter &pgw) const;
};

extern const sc_event& smoc_default_event_abort();

template <typename T_chan_kind, typename T_data_type>
class smoc_chan_if
  : public smoc_chan_in_if<T_data_type>,
    public smoc_chan_out_if<T_data_type>,
    public T_chan_kind {
public:
  // typedefs
  typedef smoc_chan_if<T_chan_kind, T_data_type>  this_type;
  typedef T_data_type                             data_type;
  typedef T_chan_kind                             chan_kind;

  bool portInIsV1()  const
    { return this->is_v1_in_port; }
  bool portOutIsV1() const
    { return this->is_v1_out_port; }
protected:
  // constructor
  smoc_chan_if(const typename chan_kind::chan_init &i)
    : chan_kind(i) {}
private:
  // disabled
  const sc_event& default_event() const { return smoc_default_event_abort(); }
};

typedef std::list<smoc_root_chan *> smoc_chan_list;

#include <smoc_port.hpp>

#endif // _INCLUDED_SMOC_CHAN_IF
