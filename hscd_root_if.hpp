// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_IF
#define _INCLUDED_HSCD_ROOT_IF

#include <systemc.h>
#include <hscd_root_port.hpp>

const sc_event& hscd_default_event_abort();

template <typename T>
class hscd_port_in;
template <typename T>
class hscd_port_out;

template <typename T>
class hscd_root_in_if
  : virtual public sc_interface
{
public:
  // typedefs
  typedef T                    data_type;
  typedef hscd_root_in_if<T>   this_type;
  typedef hscd_port_in<T>      iface_in_type;
protected:
  iface_in_type *portInIf;
public:
  // virtual const sc_event& wantEvent() const = 0;
  //  virtual void wantData( iface_type tq ) = 0;
  
  void initPortIf(iface_in_type *_i) { portInIf = _i; }
  virtual size_t committedOutCount() const = 0;
protected:  
  // constructor
  hscd_root_in_if() : portInIf(NULL) {}
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
  typedef hscd_port_out<T>      iface_out_type;
protected:
  iface_out_type *portOutIf;
public:
  // virtual const sc_event& provideEvent() const = 0;
  // virtual void provideData( iface_type tq ) = 0;
  
  void initPortIf(iface_out_type *_i) { portOutIf = _i; }
  virtual size_t committedInCount() const = 0;
protected:
  // constructor
  hscd_root_out_if() : portOutIf(NULL) {}
private:
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
  // disabled
  hscd_root_out_if( const this_type& );
  this_type& operator = ( const this_type & );
};

#endif // _INCLUDED_HSCD_ROOT_IF
