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
  bool                    ready;
  std::vector<T>          input;
  
  void clearReady() {
    input.clear();
    ready = false;
  }
 void communicate( size_t n ) {
#ifdef SYSTEMOC_DEBUG
    std::cout << "<hscd_port_in::communicate>" << std::endl;
#endif
    commSetup(n); // access to new tokens
    input.resize(n);
    for ( size_t i = 0; i < n; i++ )
      input[i] = smoc_port_in<T>::operator[](i);
    commExec(); // consume tokens
    ready = true;
#ifdef SYSTEMOC_DEBUG
    std::cout << "</hscd_port_in::communicate>" << std::endl;
#endif
  }
public:
  hscd_port_in()
    : smoc_port_in<T>()
    {}
  
  operator bool() const { return ready; }
  
  data_type &operator []( size_t n ) {
    assert( n < input.size() );
    return input[n];
  }
   
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
  bool                     ready;
  std::vector<T>           output;
  
  void clearReady() {
    ready = false;
  }
  void communicate( size_t n ) {
#ifdef SYSTEMOC_DEBUG
    std::cout << "<hscd_port_out::communicate>" << std::endl;
#endif
    commSetup(n); // access to free space on fifo
    assert( n <= output.size() );
    for ( size_t i = 0; i < n; i++ )
      smoc_port_out<T>::operator[](i) = output[i];
    output.clear();
    commExec(); // produce tokens
    ready = true;
#ifdef SYSTEMOC_DEBUG
    std::cout << "</hscd_port_out::communicate>" << std::endl;
#endif
  }
public:
  hscd_port_out()
    : smoc_port_out<T>()
    {}
  
  operator bool() const { return ready; }

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
