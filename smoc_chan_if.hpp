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

template<class T>
class smoc_channel_access {
public:
  typedef T                                              data_type;
  typedef smoc_storage<T>                                storage_type;
  typedef smoc_channel_access<data_type>                 this_type;

  virtual void   setLimit(size_t l)                    = 0;
  virtual size_t getLimit() const                      = 0;
  virtual smoc_storage<data_type>& operator[](size_t n)             = 0;
  virtual const smoc_storage<data_type> operator[](size_t n) const = 0;
};

template<>
class smoc_channel_access<void> {
public:
  typedef void                                           data_type;
  typedef smoc_channel_access<data_type>                 this_type;

  virtual void   setLimit(size_t l)                    = 0;
  virtual size_t getLimit() const                      = 0;
};

template<class S, class T>
class smoc_ring_access : public smoc_channel_access<typename S::data_type> {
public:
  typedef S					      storage_type;
  typedef T					      return_type;
  typedef smoc_ring_access<storage_type, return_type> this_type;
//private:
  storage_type *storage;
  size_t        storageSize;
  size_t       *offset;
private:
  size_t        limit;
public:
  smoc_ring_access()
    : storage(NULL), storageSize(0), offset(NULL), limit(0) {}

  void   setLimit(size_t l) { limit = l; }
  size_t getLimit() const   { return limit; }

  smoc_storage<typename S::data_type>& operator[](size_t n) {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[]" << n << ")" << std::endl;
    assert(n < limit);
    return *offset + n < storageSize
      ? storage[*offset + n]
      : storage[*offset + n - storageSize];
  }
  const smoc_storage<typename S::data_type> operator[](size_t n) const {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[](" << n << ") const" << std::endl;
    assert(n < limit);
    return *offset + n < storageSize
      ? storage[*offset + n]
      : storage[*offset + n - storageSize];
  }
};

template <>
class smoc_ring_access<void, void> : public smoc_channel_access<void> {
public:
  typedef void					      storage_type;
  typedef void					      return_type;
  typedef smoc_ring_access<storage_type, return_type> this_type;
private:
  size_t limit;
public:
  smoc_ring_access(): limit(0) {}

  void   setLimit(size_t l) { limit = l; }
  size_t getLimit() const   { return limit; }
};

template <>
class smoc_ring_access<const void, const void> : public smoc_channel_access<const void> {
public:
  typedef const void				      storage_type;
  typedef const void				      return_type;
  typedef smoc_ring_access<storage_type, return_type> this_type;
private:
  size_t limit;
public:
  smoc_ring_access(): limit(0) {}

  void   reset()            { limit = 0; }
  void   setLimit(size_t l) { limit = l; }
  size_t getLimit() const   { return limit; }
};

const sc_event& smoc_default_event_abort();

class smoc_root_chan
  : public sc_prim_channel {
public:
  // typedefs
  typedef smoc_root_chan              this_type;
  
  template <typename T_node_type,
            typename T_chan_kind,
            template <typename T_value_type> class T_chan_init_default>
  friend class smoc_graph_petri;
private:
  sc_module *hierarchy; // patched in finalise of smoc_graph_petri
public:
  virtual smoc_port_list  getInputPorts()                   const = 0;
  virtual smoc_port_list  getOutputPorts()                  const = 0;
  virtual const char     *name()                            const = 0;
  virtual void channelContents(smoc_modes::PGWriter &pgw)   const = 0;
  virtual void channelAttributes(smoc_modes::PGWriter &pgw) const = 0;
  virtual void finalise()                                         = 0;

  sc_module *getHierarchy() const {
    assert( hierarchy != NULL );  
    return hierarchy;
  }
protected:
  // constructor
  smoc_root_chan(const char *name)
    : sc_prim_channel(name), hierarchy(NULL) {}

  virtual void assemble(smoc_modes::PGWriter &pgw) const = 0;
};

class smoc_nonconflicting_chan
  : public smoc_root_chan {
public:
  // typedefs
  typedef smoc_nonconflicting_chan this_type;
protected:
  smoc_root_port_in  *portIn;
  smoc_root_port_out *portOut;

  std::string myName; // patched in finalise
public:
  // constructor
  smoc_nonconflicting_chan(const char *name)
    : smoc_root_chan(name), portIn(NULL), portOut(NULL) {}

  void addPort(smoc_root_port_in  *in)
    { /* assert(portIn  == NULL); */ portIn  = in;  }
  void addPort(smoc_root_port_out *out)
    { /* assert(portOut == NULL); */ portOut = out; }

  smoc_port_list getInputPorts()  const {
    smoc_port_list retval;
    
    assert(portIn != NULL);
    retval.push_front(portIn);
    return retval; 
  }

  smoc_port_list getOutputPorts()  const {
    smoc_port_list retval;
    
    assert(portOut != NULL);
    retval.push_front(portOut);
    return retval; 
  }

  void finalise();

  const char *name() const
    { return myName.c_str(); }
protected:
  void assemble(smoc_modes::PGWriter &pgw) const;
};

typedef std::list<smoc_root_chan *> smoc_chan_list;

template <typename T>
class smoc_port_in;
template <typename T>
class smoc_port_out;

template <typename T>
class smoc_chan_in_if
  : virtual public sc_interface {
public:
  // typedefs
  typedef T						    data_type;
  typedef smoc_chan_in_if<data_type>			    this_type;
  typedef smoc_port_in<data_type>			    iface_in_type;
  typedef typename smoc_storage_in<data_type>::storage_type storage_type;
  typedef typename smoc_storage_in<data_type>::return_type  return_type;
  typedef smoc_channel_access<data_type>                    ring_in_type;
  
  bool is_v1_in_port;
  
  virtual void   addPort(smoc_root_port_in *in) = 0;
  virtual size_t committedOutCount() const = 0;
//smoc_event &blockEventOut(size_t n) { return write_event; }
  virtual smoc_event &blockEventOut(size_t n) = 0;
  virtual ring_in_type * ringSetupIn() = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void   commExecIn(size_t consume, const smoc_ref_event_p &) = 0;
#else
  virtual void   commExecIn(size_t consume) = 0;
#endif
  virtual bool   portOutIsV1() const = 0;
  
  sc_module *getHierarchy() const {
    assert( dynamic_cast<const smoc_root_chan *>(this) != NULL );
    return dynamic_cast<const smoc_root_chan *>(this)->getHierarchy();
  }
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
  : virtual public sc_interface {
public:
  // typedefs
  typedef T						     data_type;
  typedef smoc_chan_out_if<T>				     this_type;
  typedef smoc_port_out<T>				     iface_out_type;
  typedef typename smoc_storage_out<data_type>::storage_type storage_type;
  typedef typename smoc_storage_out<data_type>::return_type  return_type;
  typedef smoc_channel_access<data_type>                     ring_out_type;
  
  bool is_v1_out_port;
  
  virtual void        addPort(smoc_root_port_out *out) = 0;
  virtual size_t      committedInCount() const = 0;
//smoc_event    &blockEventIn(size_t n) { return read_event; }
  virtual smoc_event &blockEventIn(size_t n) = 0;
  virtual ring_out_type * ringSetupOut() = 0;
#ifdef ENABLE_SYSTEMC_VPC
  virtual void        commExecOut(size_t produce, const smoc_ref_event_p &) = 0;
#else
  virtual void        commExecOut(size_t produce) = 0;
#endif
  virtual bool        portInIsV1() const = 0;
  
  sc_module *getHierarchy() const {
    assert( dynamic_cast<const smoc_root_chan *>(this) != NULL );
    return dynamic_cast<const smoc_root_chan *>(this)->getHierarchy();
  }
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

  void addPort(smoc_root_port_in  *in)
    { return T_chan_kind::addPort(in); }
  void addPort(smoc_root_port_out *out)
    { return T_chan_kind::addPort(out); }
protected:
  // constructor
  smoc_chan_if(const typename chan_kind::chan_init &i)
    : chan_kind(i) {}
private:
  // disabled
  const sc_event& default_event() const { return smoc_default_event_abort(); }
};

#include <smoc_port.hpp>

#endif // _INCLUDED_SMOC_CHAN_IF
