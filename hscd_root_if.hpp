// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_IF
#define _INCLUDED_HSCD_ROOT_IF

#include <systemc.h>
#include <hscd_root_port_list.hpp>
#include <hscd_root_port.hpp>

const sc_event& hscd_default_event_abort();

template <typename T>
class hscd_root_in_if
  : virtual public sc_interface
{
public:
  // typedefs
  typedef T                     data_type;
  typedef hscd_root_in_if<T>    this_type;
  typedef typename hscd_transfer_port<data_type>::hscd_chan_port_if
    				iface_type;
  
public:
  // virtual const sc_event& wantEvent() const = 0;
  virtual void wantData( iface_type tq ) = 0;
protected:  
  // constructor
  hscd_root_in_if()
  {}
  
  // bool askSomething( size_t n ) { return wantData(n); }
  
private:
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
  // disabled
  hscd_root_in_if( const this_type& );
  this_type &operator = ( const this_type & );
};

template <typename T>
class hscd_root_out_if
  : virtual public sc_interface
{
public:
  // typedefs
  typedef T                     data_type;
  typedef hscd_root_out_if<T>   this_type;
  typedef typename hscd_transfer_port<data_type>::hscd_chan_port_if
    				iface_type;
  
public:
  // virtual const sc_event& provideEvent() const = 0;
  virtual void provideData( iface_type tq ) = 0;
protected:
  // constructor
  hscd_root_out_if()
  {}
  
  // bool askSomething( size_t n ) { return provideData(n); }
  
private:
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
  // disabled
  hscd_root_out_if( const this_type& );
  this_type& operator = ( const this_type & );
};

#endif // _INCLUDED_HSCD_ROOT_IF
