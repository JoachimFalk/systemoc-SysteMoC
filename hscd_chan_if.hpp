// vim: set sw=2 ts=8:

#include <hscd_root_port.hpp>
#include <hscd_port.hpp>

#ifndef _INCLUDED_HSCD_CHAN_IF
#define _INCLUDED_HSCD_CHAN_IF

#include <systemc.h>

#include <list>

const sc_event& hscd_default_event_abort();

class hscd_root_chan
  : public sc_prim_channel {
public:
  // typedefs
  typedef hscd_root_chan              this_type;
  
  virtual hscd_port_list getInputPorts()  const = 0;
  virtual hscd_port_list getOutputPorts() const = 0;
protected:
  // constructor
  hscd_root_chan( const char *name)
    : sc_prim_channel(name) {}
};

template <typename T>
class hscd_port_in;
template <typename T>
class hscd_port_out;

template <typename T>
class hscd_chan_in_if
  : virtual public sc_interface {
public:
  // typedefs
  typedef T                           data_type;
  typedef hscd_chan_in_if<T>          this_type;
  typedef hscd_port_in<T>             iface_in_type;
  // virtual const sc_event& wantEvent() const = 0;
  //  virtual void wantData( iface_type tq ) = 0;
  
  virtual void   addPortIf(iface_in_type *_i) = 0;
  virtual size_t committedOutCount() const = 0;
  virtual size_t maxCommittableOutCount() const = 0;
protected:  
  // constructor
  hscd_chan_in_if() {}
private:
  // disabled
  const sc_event& default_event() const = 0;
  // disabled
  hscd_chan_in_if( const this_type& );
  this_type &operator = ( const this_type & );
};

template <typename T>
class hscd_chan_out_if
  : virtual public sc_interface {
public:
  // typedefs
  typedef T                           data_type;
  typedef hscd_chan_out_if<T>         this_type;
  typedef hscd_port_out<T>            iface_out_type;
  // virtual const sc_event& wantEvent() const = 0;
  //  virtual void wantData( iface_type tq ) = 0;
  
  virtual void   addPortIf(iface_out_type *_i) = 0;
  virtual size_t committedInCount() const = 0;
  virtual size_t maxCommittableInCount() const = 0;
protected:
  // constructor
  hscd_chan_out_if() {}
private:
  // disabled
  const sc_event& default_event() const = 0;
  // disabled
  hscd_chan_out_if( const this_type& );
  this_type& operator = ( const this_type & );
};

template <typename T_chan_kind, typename T_data_type>
class hscd_chan_if
  : public hscd_chan_in_if<T_data_type>,
    public hscd_chan_out_if<T_data_type>,
    public T_chan_kind {
public:
  // typedefs
  typedef hscd_chan_if<T_chan_kind, T_data_type>  this_type;
  
protected:
  // constructor
  hscd_chan_if(const typename T_chan_kind::chan_init &i)
    : T_chan_kind(i) {}
private:
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
};

template <typename T_chan_kind, typename T_data_type>
class hscd_chan_nonconflicting_if
  : public hscd_chan_if<T_chan_kind, T_data_type> {
public:
  // typedefs
  typedef hscd_chan_nonconflicting_if<T_chan_kind, T_data_type> this_type;
  typedef typename this_type::iface_in_type                     iface_in_type;
  typedef typename this_type::iface_out_type                    iface_out_type;
protected:
  iface_in_type  *portInIf;
  iface_out_type *portOutIf;
public:
  void addPortIf(iface_in_type *_i)
    { assert( portInIf == NULL );  portInIf = _i;  }
  void addPortIf(iface_out_type *_i)
    { assert( portOutIf == NULL ); portOutIf = _i; }
  hscd_port_list getInputPorts()  const
    { hscd_port_list retval; retval.push_front(portInIf); return retval; }
  hscd_port_list getOutputPorts() const
    { hscd_port_list retval; retval.push_front(portOutIf); return retval; }
protected:
  // constructor
  hscd_chan_nonconflicting_if(const typename T_chan_kind::chan_init &i)
    : hscd_chan_if<T_chan_kind, T_data_type>(i),
      portInIf(NULL), portOutIf(NULL) {}
};

#endif // _INCLUDED_HSCD_CHAN_IF
