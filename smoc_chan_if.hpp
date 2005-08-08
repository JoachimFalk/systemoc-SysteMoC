// vim: set sw=2 ts=8:

#include <smoc_root_port.hpp>
#include <smoc_port.hpp>

#ifndef _INCLUDED_SMOC_CHAN_IF
#define _INCLUDED_SMOC_CHAN_IF

#include <systemc.h>

#include <list>

template <typename T>
class smoc_ring_access {
public:
  typedef T                   value_type;
  typedef smoc_ring_access<T> this_type;
private:
  T       *p1;
  size_t   boundary;
  T       *p2;
  size_t   limit;
public:
  smoc_ring_access() { reset(); }
  smoc_ring_access( T *base, size_t size, size_t pos, size_t limit )
    : p1(base + pos), boundary(size-pos), p2(base - boundary), limit(limit)
    { assert( pos < size ); assert( limit <= size ); }
  
  size_t getLimit() const { return limit; }
  
  void reset() { p1 = NULL; boundary = 0; p2 = NULL; limit = 0; }
  
  T       &operator[](size_t n)
    { assert(n < limit); return n >= boundary ? p2[n] : p1[n]; }
  const T &operator[](size_t n) const
    { assert(n < limit); return n >= boundary ? p2[n] : p1[n]; }
};

class smoc_ring_access<void> {
public:
  typedef void                    value_type;
  typedef smoc_ring_access<void>  this_type;
private:
public:
  smoc_ring_access()
    {}
  smoc_ring_access( void *base, size_t size, size_t pos, size_t limit )
    { assert( pos < size ); assert( limit <= size ); assert( base == NULL ); }
};

class smoc_ring_access<const void> {
public:
  typedef void                          value_type;
  typedef smoc_ring_access<const void>  this_type;
private:
public:
  smoc_ring_access()
    {}
  smoc_ring_access( const void *base, size_t size, size_t pos, size_t limit )
    { assert( pos < size ); assert( limit <= size ); assert( base == NULL ); }
};

const sc_event& smoc_default_event_abort();

class smoc_root_chan
  : public sc_prim_channel {
public:
  // typedefs
  typedef smoc_root_chan              this_type;
  
  virtual smoc_port_list getInputPorts()  const = 0;
  virtual smoc_port_list getOutputPorts() const = 0;
protected:
  // constructor
  smoc_root_chan( const char *name)
    : sc_prim_channel(name) {}
};

template <typename T>
class smoc_port_in;
template <typename T>
class smoc_port_out;

template <typename T>
class smoc_chan_in_if
  : virtual public sc_interface {
public:
  // typedefs
  typedef T                           data_type;
  typedef smoc_chan_in_if<T>          this_type;
  typedef smoc_port_in<T>             iface_in_type;
  
  virtual void   addPortIf(iface_in_type *_i) = 0;
  virtual size_t committedOutCount() const = 0;
  virtual smoc_ring_access<const T> commSetupIn(size_t req) = 0;
  virtual void commExecIn(const smoc_ring_access<const T> &) = 0;
protected:  
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
  typedef T                           data_type;
  typedef smoc_chan_out_if<T>         this_type;
  typedef smoc_port_out<T>            iface_out_type;
  
  virtual void   addPortIf(iface_out_type *_i) = 0;
  virtual size_t committedInCount() const = 0;
  virtual smoc_ring_access<T> commSetupOut(size_t req) = 0;
  virtual void commExecOut(const smoc_ring_access<T> &) = 0;
protected:
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
  // constructor
  smoc_chan_nonconflicting_if(const typename T_chan_kind::chan_init &i)
    : smoc_chan_if<T_chan_kind, T_data_type>(i),
      portInIf(NULL), portOutIf(NULL) {}
};

#endif // _INCLUDED_SMOC_CHAN_IF
