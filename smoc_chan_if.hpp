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
  storage_type *p1;
  size_t	boundary;
  storage_type *p2;
  size_t	limit;
public:
  smoc_ring_access()
    { reset(); }
  
  smoc_ring_access(storage_type *base, size_t size, size_t pos, size_t limit)
    : p1(base + pos), boundary(size-pos), p2(base - boundary), limit(limit)
    { assert( pos < size ); assert( limit <= size ); }
  
  size_t getLimit() const
    { return limit; }
  
  void reset()
    { p1 = NULL; boundary = 0; p2 = NULL; limit = 0; }
  
  return_type operator[](size_t n) {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[]" << n << ")" << std::endl;
    assert(n < limit);
    return n >= boundary ? p2[n] : p1[n];
  }
  const return_type operator[](size_t n) const {
    // std::cerr << "((smoc_ring_access)" << this << ")->operator[](" << n << ") const" << std::endl;
    assert(n < limit);
    return n >= boundary ? p2[n] : p1[n];
  }
};

template <>
class smoc_ring_access<void, void> {
public:
  typedef void					      storage_type;
  typedef void					      return_type;
  typedef smoc_ring_access<storage_type, return_type> this_type;
private:
  size_t limit;
public:
  smoc_ring_access()
    { reset(); }
  
  smoc_ring_access(storage_type *base, size_t size, size_t pos, size_t limit)
    : limit(limit)
    { assert( pos < size ); assert( limit <= size ); assert( base == NULL ); }
  
  size_t getLimit() const
    { return limit; }
  
  void reset()
    { limit = 0; }
};

template <>
class smoc_ring_access<const void, const void> {
public:
  typedef const void				      storage_type;
  typedef const void				      return_type;
  typedef smoc_ring_access<storage_type, return_type> this_type;
private:
  size_t limit;
public:
  smoc_ring_access()
    { reset(); }
  
  smoc_ring_access(storage_type *base, size_t size, size_t pos, size_t limit)
    : limit(limit)
    { assert( pos < size ); assert( limit <= size ); assert( base == NULL ); }

  size_t getLimit() const
    { return limit; }
  
  void reset()
    { limit = 0; }
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
  sc_module *hierarchy; // patched in finalize of smoc_graph_petri
public:
  virtual smoc_port_list  getInputPorts()               const = 0;
  virtual smoc_port_list  getOutputPorts()              const = 0;
  virtual void            assemble(
                            smoc_modes::PGWriter &pgw)  const = 0;
  
  sc_module *getHierarchy() const {
    assert( hierarchy != NULL );  
    return hierarchy;
  }
protected:
  // constructor
  smoc_root_chan( const char *name)
    : sc_prim_channel(name), hierarchy(NULL) {}
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
  typedef smoc_ring_access<storage_type, return_type>	    ring_type;
  
  bool is_v1_in_port;
  
  virtual void addPortIf(iface_in_type *_i) = 0;
  virtual size_t committedOutCount() const = 0;
  smoc_event &blockEventOut() { return write_event; }
  virtual ring_type commSetupIn(size_t req) = 0;
  virtual void commExecIn(const ring_type &) = 0;
  virtual bool portOutIsV1() const = 0;
  
  sc_module *getHierarchy() const {
    assert( dynamic_cast<const smoc_root_chan *>(this) != NULL );
    return dynamic_cast<const smoc_root_chan *>(this)->getHierarchy();
  }
protected:
  smoc_event write_event;
  
  // constructor
  smoc_chan_in_if() {}
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
  typedef smoc_ring_access<storage_type, return_type>	     ring_type;
  
  bool is_v1_out_port;
  
  virtual void   addPortIf(iface_out_type *_i) = 0;
  virtual size_t committedInCount() const = 0;
  smoc_event    &blockEventIn() { return read_event; }
  virtual ring_type commSetupOut(size_t req) = 0;
  virtual void commExecOut(const ring_type &) = 0;
  virtual bool portInIsV1() const = 0;
  
  sc_module *getHierarchy() const {
    assert( dynamic_cast<const smoc_root_chan *>(this) != NULL );
    return dynamic_cast<const smoc_root_chan *>(this)->getHierarchy();
  }
protected:
  smoc_event read_event;
  
  // constructor
  smoc_chan_out_if() {}
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
  
  bool portInIsV1()  const
    { return this->is_v1_in_port; }
  bool portOutIsV1() const
    { return this->is_v1_out_port; }
protected:
  // constructor
  smoc_chan_if(const typename T_chan_kind::chan_init &i)
    : T_chan_kind(i) {}
private:
  // disabled
  const sc_event& default_event() const { return smoc_default_event_abort(); }
};

template <typename T_chan_kind, typename T_data_type>
class smoc_chan_nonconflicting_if
  : public smoc_chan_if<T_chan_kind, T_data_type> {
public:
  // typedefs
  typedef smoc_chan_nonconflicting_if<T_chan_kind, T_data_type> this_type;
  typedef typename this_type::iface_in_type                     iface_in_type;
  typedef typename this_type::iface_out_type                    iface_out_type;
protected:
  iface_in_type  *portInIf;
  iface_out_type *portOutIf;
public:
  void addPortIf(iface_in_type *_i)
    { /*assert( portInIf == NULL );*/  portInIf = _i;  }
  void addPortIf(iface_out_type *_i)
    { /*assert( portOutIf == NULL );*/ portOutIf = _i; }
  smoc_port_list getInputPorts()  const
    { smoc_port_list retval; retval.push_front(portInIf); return retval; }
  smoc_port_list getOutputPorts() const
    { smoc_port_list retval; retval.push_front(portOutIf); return retval; }
protected:
  void assemble(smoc_modes::PGWriter &pgw) const {
    assert(portInIf != NULL && portOutIf != NULL);
    
    std::string idChannel        = pgw.getId(this);
    std::string idChannelPortIn  = pgw.getId(reinterpret_cast<const char *>(this)+1);
    std::string idChannelPortOut = pgw.getId(reinterpret_cast<const char *>(this)+2);
    
    pgw << "<edge name=\""   << this->name() << ".to-edge\" "
                 "source=\"" << pgw.getId(portOutIf) << "\" "
                 "target=\"" << idChannelPortIn      << "\" "
                 "id=\""     << pgw.getId()          << "\"/>" << std::endl;
    pgw << "<process name=\"" << this->name() << "\" "
                    "type=\"fifo\" "
                    "id=\"" << idChannel      << "\">" << std::endl;
    {
      pgw.indentUp();
      pgw << "<port name=\"" << this->name() << ".in\" "
                   "type=\"in\" "
                   "id=\"" << idChannelPortIn << "\"/>" << std::endl;
      pgw << "<port name=\"" << this->name() << ".out\" "
                   "type=\"out\" "
                   "id=\"" << idChannelPortOut << "\"/>" << std::endl;
      //*******************************ACTOR CLASS********************************
      pgw << "<fifo tokenType=\"" << typeid(T_data_type).name() << "\">" << std::endl;
      {
        //*************************INITIAL TOKENS, ETC...***************************
        pgw.indentUp();
        this->channelContents(pgw);
        pgw.indentDown();
      }
      pgw << "</fifo>" << std::endl;
      this->channelAttributes(pgw); // fifo size, etc..
      pgw.indentDown();
    }
    pgw << "</process>" << std::endl;
    pgw << "<edge name=\""   << this->name() << ".from-edge\" "
                 "source=\"" << idChannelPortOut       << "\" "
                 "target=\"" << pgw.getId(portInIf)    << "\" "
                 "id=\""     << pgw.getId()            << "\"/>" << std::endl;
  }

  // constructor
  smoc_chan_nonconflicting_if(const typename T_chan_kind::chan_init &i)
    : smoc_chan_if<T_chan_kind, T_data_type>(i),
      portInIf(NULL), portOutIf(NULL) {}
};

#endif // _INCLUDED_SMOC_CHAN_IF
