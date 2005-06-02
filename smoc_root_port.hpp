// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_ROOT_PORT_HPP
#define _INCLUDED_SMOC_ROOT_PORT_HPP

#include <iostream>
#include <cassert>

#include <list>

#include <systemc.h>

class smoc_root_port
  : protected sc_port_base {
public:
  friend class smoc_op_port;
  
  typedef smoc_root_port  this_type;
private:
  size_t  committed;
  size_t  done;
//  size_t  maxcommittable;
  bool    uplevel;
protected:
  smoc_root_port( const char* name_ )
    : sc_port_base( name_, 1 ), uplevel(false) { reset(); }
  
  virtual void transfer() = 0;
public:
  static const char* const kind_string;
  virtual const char* kind() const
    { return kind_string; }
  
  virtual bool isInput() const = 0;
  bool isOutput() const { return !isInput(); }
  bool isUplevel() const { return uplevel; }
  
  virtual void reset() { committed = done = 0; /*maxcommittable = ~0;*/ }
  
  bool setCommittedCount( size_t _committed ) {
    assert( _committed >= committed /*&&
            _committed <= maxcommittable*/ );
    bool retval = committed != _committed;
    committed = _committed;
    return retval;
  }
//  bool setMaxCommittable( size_t _maxcommittable ) {
//    assert( _maxcommittable <= maxcommittable &&
//            _maxcommittable >= committed );
//    bool retval = maxcommittable != _maxcommittable;
//    maxcommittable = _maxcommittable;
//    return retval;
//  }
  bool canTransfer() const { return committedCount() > doneCount(); }
  operator bool() const { return !canTransfer(); }
  
  size_t doneCount() const { return done; }
  
  size_t committedCount() const { return committed; }
  virtual size_t availableCount() const = 0;
//  size_t maxCommittableCount() const { return maxcommittable; }
//  virtual size_t maxAvailableCount() const = 0;
  
  size_t incrDoneCount() { return done++; }

  // bind interface to this port
  void bind( sc_interface& interface_ ) { sc_port_base::bind(interface_); }
  // bind parent port to this port
  void bind( this_type &parent_ ) { uplevel = true; sc_port_base::bind(parent_); }
private:
  // disabled
  smoc_root_port( const this_type & );
  this_type& operator = ( const this_type & );
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_root_port &p ) {
  out << "port(" << &p << ","
           "uplevel=" << p.isUplevel() << ","
           "committed=" << p.committedCount() << ","
//           "maxcommittable=" << p.maxCommittableCount() << ","
           "available=" << p.availableCount() << ")";
  return out;
}

typedef std::list<smoc_root_port *> smoc_port_list;

class smoc_op_port {
public:
  typedef smoc_op_port  this_type;
  
  friend class smoc_activation_pattern;
  friend class smoc_firing_state;
  friend class smoc_port_tokens;
private:
  smoc_root_port *port;
  size_t          commit;
protected:
  bool stillPossible() const {
    return (commit >= port->committedCount()) /*&&
           (commit <= port->maxCommittableCount())*/;
  }
  
  smoc_op_port( smoc_root_port *port, size_t commit )
    : port(port), commit(commit) {}
  
  void                  addCommitCount( size_t n ) { commit += n; }
public:
  size_t                commitCount() const { return commit; }
  
  smoc_root_port       *getPort()           { return port; }
  const smoc_root_port *getPort()     const { return port; }
  
  bool knownUnsatisfiable() const
    { return /*commit  > port->maxAvailableCount() ||*/ !stillPossible(); }
  bool knownSatisfiable()  const
    { return commit <= port->availableCount() && stillPossible(); }
  bool satisfied()   const
    { return commit == port->doneCount() && stillPossible(); }
  bool isInput()     const { return port->isInput();  }
  bool isOutput()    const { return port->isOutput(); }
  bool isUplevel()   const { return port->isUplevel(); }
  
  void reset()    { port->reset(); }
  void transfer() { port->setCommittedCount(commit); port->transfer(); }
};

template <typename T> class smoc_port_in;
template <typename T> class smoc_port_out;

class smoc_port_tokens {
public:
  typedef smoc_port_tokens  this_type;
  
  template <typename T> friend class smoc_port_in;
  template <typename T> friend class smoc_port_out;
private:
  smoc_root_port *port;
protected:
  smoc_port_tokens(smoc_root_port *port)
    : port(port) {}
public:
  class smoc_op_port operator >=(size_t n)
    { return smoc_op_port(port,n); }
  class smoc_op_port operator > (size_t n)
    { return *this >= n+1; }
};

#endif // _INCLUDED_SMOC_ROOT_PORT_HPP
