// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_POPT_HPP
#define _INCLUDED_HSCD_POPT_HPP

#include <hscd_root_port_list.hpp>
#include <smoc_port.hpp>

template<typename T>
class hscd_port_in
  : public smoc_port_in<T> {
public:
  typedef T                               data_type;
  typedef hscd_port_in<T>                 this_type;
  typedef typename this_type::iface_type  iface_type;
private:
  std::vector<T>          input;
  
/*
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
 */
public:
  hscd_port_in()
    : smoc_port_in<T>()
    {}

  operator bool() const { return false; }
  
  hscd_op_port operator ()( size_t n )
    { return hscd_op_port(this,n); }
  void operator () ( iface_type& interface_ )
    { bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};

template<typename T>
class hscd_port_out
  : public smoc_port_out<T> {
public:
  typedef T                               data_type;
  typedef hscd_port_out<T>                this_type;
  typedef typename this_type::iface_type  iface_type;
private:
  std::vector<T>           output;
 
  /*
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
  }*/
  
public:
  hscd_port_out()
    : smoc_port_out<T>()
    {}
  
  operator bool() const { return false; }

  hscd_op_port operator ()( size_t n )
    { return hscd_op_port(this,n); }
  void operator () ( iface_type& interface_ )
    { bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
  
  data_type &operator []( size_t n ) {
    if ( n >= output.size() )
      output.resize(n+1);
    return output[n];
  }
};

#endif // _INCLUDED_HSCD_POPT_HPP
