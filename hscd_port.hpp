// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_POPT_HPP
#define _INCLUDED_HSCD_POPT_HPP

#include <hscd_root_port.hpp>
#include <hscd_root_if.hpp>
#include <systemc.h>

template<typename T>
class hscd_port_in
  : public hscd_transfer_port<T>,
    public sc_port<hscd_root_in_if<T> > {
public:
  typedef T                  data_type;
  typedef hscd_port_in<T>    this_type;
  typedef hscd_root_in_if<T> iface_type;
private:
  std::vector<T>          input;
  
  void requestTransfer( hscd_port2op_if &op, size_t n ) {
    input.resize(n);
    addr  = static_cast<void *>(&input.front());
    count_left  = input.size();
    count_start = count_left;
    (*this)->wantData( chanCallback() );
    if ( ready() )
      finishTransfer();
    else
      runningOp = &op;
  }
  
public:
  hscd_port_in()
    : sc_port<hscd_root_in_if<T> >( sc_gen_unique_name( "hscd_port_in" )  )
    { isInput = true; }
  
  class hscd_op_port operator ()( size_t n ) {
    return setTokens(n);
  }
  void operator () ( iface_type& interface_ ) {
    return sc_port<iface_type>::operator()(interface_);
  }
  void operator () ( this_type& parent_ ) {
    return sc_port<iface_type>::operator()(parent_);
  }
  
  data_type &operator []( size_t n ) {
    assert( n < input.size() );
    return input[n];
  }
};

class hscd_port_in<void>
  : public hscd_transfer_port<void>,
    public sc_port<hscd_root_in_if<void> > {
public:
  typedef void                  data_type;
  typedef hscd_port_in<void>    this_type;
  typedef hscd_root_in_if<void> iface_type;
private:
  void requestTransfer( hscd_port2op_if &op, size_t n ) {
    count_left  = n;
    count_start = count_left;
    (*this)->wantData( chanCallback() );
    if ( ready() )
      finishTransfer();
    else
      runningOp = &op;
  }
  
public:
  hscd_port_in()
    : sc_port<hscd_root_in_if<void> >( sc_gen_unique_name( "hscd_port_in" )  )
    { isInput = true; }
  
  class hscd_op_port operator ()( size_t n ) {
    return setTokens(n);
  }
  void operator () ( iface_type& interface_ ) {
    return sc_port<iface_type>::operator()(interface_);
  }
  void operator () ( this_type& parent_ ) {
    return sc_port<iface_type>::operator()(parent_);
  }
};

template<typename T>
class hscd_port_out
  : public hscd_transfer_port<T>,
    public sc_port<hscd_root_out_if<T> > {
public:
  typedef T                   data_type;
  typedef hscd_port_out<T>    this_type;
  typedef hscd_root_out_if<T> iface_type;
private:
  std::vector<T>           output;
  
  void requestTransfer( hscd_port2op_if &op, size_t n ) {
    assert( n <= output.size() );
    addr  = static_cast<void *>(&output.front());
    count_left  = output.size();
    count_start = count_left;
    (*this)->provideData( chanCallback() );
    if ( ready() )
      finishTransfer();
    else
      runningOp = &op;
  }
  
public:
  hscd_port_out()
    : sc_port<hscd_root_out_if<T> >( sc_gen_unique_name( "hscd_port_out" )  )
    { isInput = false; }
  
  
  class hscd_op_port operator ()( size_t n ) {
    return setTokens(n);
  }
  void operator () ( iface_type& interface_ ) {
    return sc_port<iface_type>::operator()(interface_);
  }
  void operator () ( this_type& parent_ ) {
    return sc_port<iface_type>::operator()(parent_);
  }
  
  data_type &operator []( size_t n ) {
    if ( n >= output.size() )
      output.resize(n+1);
    return output[n];
  }
};

class hscd_port_out<void>
  : public hscd_transfer_port<void>,
    public sc_port<hscd_root_out_if<void> > {
public:
  typedef void                   data_type;
  typedef hscd_port_out<void>    this_type;
  typedef hscd_root_out_if<void> iface_type;
private:
  void requestTransfer( hscd_port2op_if &op, size_t n ) {
    count_left  = n;
    count_start = count_left;
    (*this)->provideData( chanCallback() );
    if ( ready() )
      finishTransfer();
    else
      runningOp = &op;
  }
  
public:
  hscd_port_out()
    : sc_port<hscd_root_out_if<void> >( sc_gen_unique_name( "hscd_port_out" )  )
    { isInput = false; }
  
  
  class hscd_op_port operator ()( size_t n ) {
    return setTokens(n);
  }
  void operator () ( iface_type& interface_ ) {
    return sc_port<iface_type>::operator()(interface_);
  }
  void operator () ( this_type& parent_ ) {
    return sc_port<iface_type>::operator()(parent_);
  }
};

#endif // _INCLUDED_HSCD_POPT_HPP
