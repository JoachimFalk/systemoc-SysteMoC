// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_PORT_HPP
#define _INCLUDED_HSCD_ROOT_PORT_HPP

#include <iostream>
#include <cassert>

#include <list>

#include <systemc.h>

class hscd_root_port
  : protected sc_port_base {
public:
  typedef hscd_root_port  this_type;
private:
  size_t  committed;
  size_t  done;
  size_t  maxcommittable;
public:
  hscd_root_port( const char* name_ )
    : sc_port_base( name_, 1 ) { reset(); }
  
  static const char* const kind_string;
  virtual const char* kind() const
    { return kind_string; }
  
  virtual void reset() { committed = done = 0; maxcommittable = ~0; }

  bool setCommittedCount( size_t _committed ) {
    assert( _committed >= committed &&
            _committed <= maxcommittable );
    bool retval = committed != _committed;
    committed = _committed;
    return retval;
  }
  bool setMaxCommittable( size_t _maxcommittable ) {
    assert( _maxcommittable <= maxcommittable &&
            _maxcommittable >= committed );
    bool retval = maxcommittable != _maxcommittable;
    maxcommittable = _maxcommittable;
    return retval;
  }
  bool canTransfer() const { return committedCount() > doneCount(); }
  operator bool() const { return !canTransfer(); }
  
  size_t doneCount() const { return done; }
  
  size_t committedCount() const { return committed; }
  virtual size_t availableCount() const = 0;
  size_t maxCommittableCount() const { return maxcommittable; }
  virtual size_t maxAvailableCount() const = 0;
  
  virtual bool isInput() const = 0;
  bool isOutput() const { return !isInput(); }
protected:
  size_t incrDoneCount() { return done++; }
private:
  // disabled
  hscd_root_port( const this_type & );
  this_type& operator = ( const this_type & );
};

static inline
std::ostream &operator <<( std::ostream &out, const hscd_root_port &p ) {
  out << "port(" << &p << ","
           "committed=" << p.committedCount() << ","
           "maxcommittable=" << p.maxCommittableCount() << ")";
  return out;
}

typedef std::list<hscd_root_port *> hscd_port_list;

#endif // _INCLUDED_HSCD_ROOT_PORT_HPP
