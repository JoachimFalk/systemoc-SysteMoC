// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_PORT_HPP
#define _INCLUDED_HSCD_ROOT_PORT_HPP

//#include <hscd_root_port_list.hpp>
//#include <assert.h>

#include <systemc.h>

#include <iostream>
#include <cassert>

class hscd_root_port {
public:
  typedef hscd_root_port  this_type;
private:
  bool    final;
  size_t  committed, done;
public:
  hscd_root_port() { reset(); }
  
  virtual void reset() { final = false; committed = 0; done = 0; }
  virtual void setCommittedCount( size_t _committed )
    { assert( committed <= _committed ); committed = _committed; }
  void finalise() { final = true; }
  
  bool canTransfer() const { return committedCount() > doneCount(); }
  operator bool() const { return !canTransfer(); }
  
  bool   isFinal() const { return final; }
  size_t doneCount() const { return done; }
  size_t committedCount() const { return committed; }
  virtual size_t availableCount() const = 0;

  virtual bool isInput() const = 0;
  bool isOutput() const { return !isInput(); }

protected:
  size_t incrDoneCount() { return done++; }
private:
  // disabled
  hscd_root_port( const this_type & );
  this_type& operator = ( const this_type & );
};

#endif // _INCLUDED_HSCD_ROOT_PORT_HPP
