// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_OP_PORT_LIST_HPP
#define _INCLUDED_HSCD_OP_PORT_LIST_HPP

#include <vector>
#include <iostream>

#include <smoc_root_port.hpp>

class hscd_op_port_list;
class hscd_op_port_or_list;
class hscd_op_port_and_list;

class hscd_port2op_if;

template<typename T> class hscd_port_in;
template<typename T> class hscd_port_out;

class hscd_op_port {
public:
  template <typename T> friend class hscd_port_in;
  template <typename T> friend class hscd_port_out;
  
  typedef hscd_op_port	this_type;
private:
  smoc_root_port       *port;
  size_t		tokens;
  
  hscd_op_port( smoc_root_port *port, size_t tokens )
    : port(port), tokens(tokens) {}
public:
  bool isInput() const
    { return port->isInput(); }
  bool isOutput() const
    { return port->isOutput(); }
  bool isReady() const
    { return port->availableCount() >= tokens; }
  smoc_event &blockEvent() const
    { return port->blockEvent(); }
  void communicate() const
    { return port->communicate(tokens); }
  void clearReady() const
    { return port->clearReady(); }
  
  hscd_op_port_or_list  operator | ( const hscd_op_port &p );
  hscd_op_port_and_list operator & ( const hscd_op_port &p );
};

class hscd_op_port_base_list
: public std::vector<hscd_op_port> {
public:
  typedef hscd_op_port_base_list this_type;
protected:
  template <typename T>
  this_type onlyXXX(const T filter) const {
    this_type retval;
    
    for ( const_iterator iter = begin();
	  iter != end();
	  ++iter ) {
      if ( filter(*iter) )
	retval.push_back( *iter );
    }
    return retval;
  }
  struct filterInput_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.isInput(); }
  };
  struct filterOutput_ty {
    bool operator ()( const hscd_op_port &port ) const
      { return port.isOutput(); }
  };
public:
  hscd_op_port_base_list()
    {}
  hscd_op_port_base_list( const hscd_op_port &p )
    { push_back(p); }
  
  this_type onlyInputs()  const { return onlyXXX(filterInput_ty()); }
  this_type onlyOutputs() const { return onlyXXX(filterOutput_ty()); }
};

class hscd_op_port_or_list
: public hscd_op_port_base_list {
public:
  typedef hscd_op_port_or_list this_type;
  
  hscd_op_port_or_list() {}
  hscd_op_port_or_list( const hscd_op_port &p )
    : hscd_op_port_base_list(p) {}
  
  this_type &operator |= ( const hscd_op_port &p )
    { push_back(p); return *this; }
  this_type operator | ( const hscd_op_port &p )
    { return this_type(*this) |= p; }
};

class hscd_op_port_and_list
: public hscd_op_port_base_list {
public:
  typedef hscd_op_port_and_list this_type;
  
  hscd_op_port_and_list() {}
  hscd_op_port_and_list( const hscd_op_port &p )
    : hscd_op_port_base_list(p) {}
  
  this_type &operator &= ( const hscd_op_port &p )
    { push_back(p); return *this; }
  this_type operator & ( const hscd_op_port &p )
    { return this_type(*this) &= p; }
};

inline
hscd_op_port_or_list  hscd_op_port::operator | ( const hscd_op_port &p )
  { return hscd_op_port_or_list(*this) |= p; }

inline
hscd_op_port_and_list hscd_op_port::operator & ( const hscd_op_port &p )
  { return hscd_op_port_and_list(*this) &= p; }

#endif // _INCLUDED_HSCD_OP_PORT_LIST_HPP
