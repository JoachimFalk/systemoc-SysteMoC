// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_ROOT_PORT_HPP
#define _INCLUDED_HSCD_ROOT_PORT_HPP

#include <hscd_root_port_list.hpp>
#include <assert.h>

class hscd_port2op_if {
  friend class hscd_root_port;
protected:
  virtual void incommingTransfer( hscd_root_port &p ) = 0;
public:
  virtual ~hscd_port2op_if() {}
};

class hscd_root_port {
  friend class hscd_root_node;
  friend class hscd_op2port_if;
  friend class hscd_op_port;
public:
  typedef hscd_root_port  this_type;
protected:
  void                   *addr;
  size_t                  count_start;
  size_t                  count_left;
  void                  **cancelPtr;
  class hscd_port2op_if  *runningOp;  
  
  virtual void requestTransfer( hscd_port2op_if &op, size_t tokens ) = 0;
  
  void notify() {
    assert( runningOp != NULL );
    return runningOp->incommingTransfer( *this );
  }
  
  void cancelTransfer() {
    assert( !started() );
    if ( cancelPtr != NULL )
      *cancelPtr = NULL;
    addr      = NULL;
    cancelPtr = NULL;
    runningOp = NULL;
    clearReady();
  }
  
  void finishTransfer() {
    assert( ready() );
    if ( cancelPtr != NULL )
      *cancelPtr = NULL;
    addr      = NULL;
    cancelPtr = NULL;
    runningOp = NULL;
  }
  
  void clearReady() { count_left = (size_t) -1; }
  
  bool started() const { return count_left != count_start; }
public:
  bool			  isInput;
  
  hscd_root_port()
    :addr(NULL), count_start(0), count_left(0),
     cancelPtr(NULL), runningOp(NULL) {}
  bool ready() const { return count_left == 0; }
  operator bool() const { return ready(); }
  
  class hscd_op_port setTokens( size_t n ) {
    return hscd_op_port(*this,n);
  }
private:
  // disabled
  hscd_root_port( const this_type & );
  this_type& operator = ( const this_type & );
};

inline
void hscd_op_port::requestTransfer( hscd_port2op_if &op ) {
  return port->requestTransfer( op, tokens );
}

inline
bool hscd_op_port::isInput() const { return port->isInput; }

class hscd_op2port_if {
protected:
  void requestTransfer( hscd_op_port &p, hscd_port2op_if &callback ) {
    return p.requestTransfer(callback);
  }
  void cancelTransfer( hscd_root_port &p ) {
    return p.cancelTransfer();
  }
  void finishTransfer( hscd_root_port &p ) {
    return p.finishTransfer();
  }
  void clearReady( hscd_root_port &p ) {
    return p.clearReady();
  }
  bool ready( hscd_root_port &p ) const {
    return p.ready();
  }
  bool started( hscd_root_port &p ) const {
    return p.started();
  }
  bool inProgress( hscd_root_port &p ) const {
    return !ready(p) && started(p);
  }
};

class hscd_port_op_if
  : public hscd_port2op_if,
    public hscd_op2port_if {
protected:
  void requestTransfer( hscd_op_port &p ) {
    return hscd_op2port_if::requestTransfer(p,*this);
  }
};

template <typename T>
class hscd_transfer_port
  :public hscd_root_port {
  friend class hscd_chan_port_if;
public:
  typedef T                     data_type;
  typedef hscd_transfer_port<T> this_type;
  
  class hscd_chan_port_if {
    friend class hscd_transfer_port<T>;
  public:
    typedef T                     data_type;
    typedef hscd_chan_port_if     this_type;
  private:
    hscd_transfer_port<data_type> *port;
    
    hscd_chan_port_if( hscd_transfer_port<data_type> *port )
      :port(port) {}
  public:
    hscd_chan_port_if()
      :port(NULL) {}
    
    bool haveRequest() { return port != NULL; }
    void notify() { return port->notify(); }
    void setCancler() { return port->cancler(&port); }
    bool ready() { return port->ready(); }
    data_type *nextAddr() { return port->nextAddr(); }
  };
protected:
  void cancler( this_type **ptr ) {
    cancelPtr = reinterpret_cast<void **>(ptr);
  }
  
  data_type *nextAddr();
  
  hscd_chan_port_if chanCallback() {
    return hscd_chan_port_if(this);
  }
public:
  hscd_transfer_port() {}
  
private:
  // disabled
  hscd_transfer_port( const this_type & );
  this_type& operator = ( const this_type & );
};

template <typename T>
T *hscd_transfer_port<T>::nextAddr() {
  assert( !ready() );
  count_left--;
  return reinterpret_cast<data_type *>(addr)++;
}

inline
void *hscd_transfer_port<void>::nextAddr() {
  assert( !ready() );
  count_left--;
  return NULL;
}

#endif // _INCLUDED_HSCD_ROOT_PORT_HPP
