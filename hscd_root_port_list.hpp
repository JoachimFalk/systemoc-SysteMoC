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
  template <typename T> friend class hscd_port_in;
  template <typename T> friend class hscd_port_out;
public:
  typedef hscd_op_port	this_type;
private:
  smoc_root_port       *port;
  size_t		tokens;
  
  void requestTransfer( hscd_port2op_if &op );
  
  hscd_op_port( smoc_root_port *port, size_t tokens )
    : port(port), tokens(tokens) {}
public:
  operator smoc_root_port &() { return *port; }
  
  bool isInput() const  { return port->isInput(); }
  bool isOutput() const { return port->isOutput(); }
  
  operator hscd_op_port_list &();
  hscd_op_port_or_list  &operator | ( hscd_op_port p );
  hscd_op_port_and_list &operator & ( hscd_op_port p );
};

class hscd_op_port_base_list
  :public std::vector<hscd_op_port> {
public:
  typedef hscd_op_port_base_list this_type;
  
protected:
  hscd_op_port_base_list() {}
  
  template <typename T>
  this_type &onlyXXX(const T filter) const {
    this_type *retval = new hscd_op_port_base_list();
    
    for ( const_iterator iter = begin();
	  iter != end();
	  ++iter ) {
      if ( filter(*iter) )
	retval->push_back( *iter );
    }
    return *retval;
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
  this_type &onlyInputs() const { return onlyXXX(filterInput_ty()); }
  this_type &onlyOutputs() const { return onlyXXX(filterOutput_ty()); }
};

class hscd_op_port_or_list
  :public virtual hscd_op_port_base_list {
public:
  typedef hscd_op_port_or_list this_type;
  
  hscd_op_port_or_list() {}
  
  this_type &operator | ( hscd_op_port p ) {
    push_back(p); return *this;
  }
};

class hscd_op_port_and_list
  :public virtual hscd_op_port_base_list {
public:
  typedef hscd_op_port_and_list this_type;
  
  hscd_op_port_and_list() {}
  
  this_type &operator & ( hscd_op_port p ) {
    push_back(p); return *this;
  }
};

class hscd_op_port_list
  :public hscd_op_port_or_list,
   public hscd_op_port_and_list {
public:
  friend class hscd_op_port;
  typedef hscd_op_port_list this_type;
  
protected:
  hscd_op_port_list( hscd_op_port p ) {
    push_back(p);
  }
};

inline
hscd_op_port::operator hscd_op_port_list &() {
  return *new hscd_op_port_list(*this);
}

inline
hscd_op_port_or_list  &hscd_op_port::operator | ( hscd_op_port p ) {
  return static_cast<hscd_op_port_list &>(*this) | p;
}

inline
hscd_op_port_and_list &hscd_op_port::operator & ( hscd_op_port p ) {
  return static_cast<hscd_op_port_list &>(*this) & p;
}


#endif // _INCLUDED_HSCD_OP_PORT_LIST_HPP
