// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_POPT_HPP
#define _INCLUDED_HSCD_POPT_HPP

#include <hscd_root_port.hpp>
#include <hscd_root_if.hpp>
#include <systemc.h>
#include <vector>

template <typename T>
class hscd_port_base
  : protected sc_port_base {
public:
  typedef T                   iface_type;
  typedef hscd_port_base<T>   this_type;
private:
  typedef sc_port_base        base_type;
  
  iface_type  *interface;
  
  const char *if_typename() const { return typeid(iface_type).name(); }
protected:

  // called by pbind (for internal use only)
  int vbind( sc_interface& interface_ ) {
      iface_type *iface = dynamic_cast<iface_type *>( &interface_ );
      if( iface == 0 ) {
          // type mismatch
          return 2;
      }
      base_type::bind( *iface );
      return 0;
  }
  int vbind( sc_port_base& parent_ ) {
      this_type* parent = dynamic_cast<this_type*>( &parent_ );
      if( parent == 0 ) {
          // type mismatch
          return 2;
      }
      base_type::bind( *parent );
      return 0;
  }
  void push_interface( sc_interface *_i ) {
    assert( interface == NULL );
    interface = dynamic_cast<iface_type *>(_i);
    assert( interface != NULL );
  }
public:
  hscd_port_base( const char* name_ )
    : base_type( name_, 1 ), interface( NULL ) {}
  
  // get the first interface without checking for nil
  sc_interface       *get_interface()       { return interface; }
  sc_interface const *get_interface() const { return interface; }
  
  iface_type       *operator -> () {
    if ( interface == NULL )
      report_error( SC_ID_GET_IF_, "port is not bound" );
    return interface;
  }
  
  iface_type const *operator -> () const {
    if ( interface == NULL )
      report_error( SC_ID_GET_IF_, "port is not bound" );
    return interface;
  }
};

template <typename T>
class hscd_port_storage_in
  : protected hscd_port_base<hscd_root_in_if<T> > {
public:
  typedef T    data_type;
private:
  std::vector<T> s;
protected:
  void storageClear() { s.clear(); }
  void storagePushBack( const data_type *in ) { s.push_back(*in); }

  hscd_port_storage_in( const char *n )
    : hscd_port_base<hscd_root_in_if<data_type> >(n) {}
public:
  data_type &operator []( size_t n )
    { assert( n < s.size() ); return s[n]; }
};

class hscd_port_storage_in<void>
  : protected hscd_port_base<hscd_root_in_if<void> > {
public:
  typedef void data_type;
private:
protected:
  void storageClear() {}
  void storagePushBack( const data_type *in ) { assert(in == NULL); }
  
  hscd_port_storage_in( const char *n )
    : hscd_port_base<hscd_root_in_if<data_type> >(n) {}
public:
};

template <typename T>
class hscd_port_in
  : public hscd_root_port,
    public hscd_port_storage_in<T> {
public:
  typedef T                     data_type;
  typedef hscd_port_in<T>       this_type;
  typedef typename this_type::iface_type iface_type;
private:
//  void setCommittedCount( size_t _committed ) {
//    return hscd_root_port::setCommittedCount(_committed);
//  }
  
  void reset() {
    storageClear(); return hscd_root_port::reset();
  }
  
  void add_interface( sc_interface *i ) {
    push_interface(i); (*this)->initPortIf( this );
  }
public:
  void transferIn( const T *in ) { storagePushBack(in); incrDoneCount(); }
//public:
  hscd_port_in()
    : hscd_port_storage_in<T>( sc_gen_unique_name( "hscd_port_in" )  )
    {}
  
  bool isInput() const { return true; }

  size_t availableCount() const { return doneCount() + (*this)->committedOutCount(); }
  
  class hscd_op_port operator ()( size_t n ) {
    return hscd_op_port(this,n);
  }
  void operator () ( iface_type& interface_ ) { bind(interface_); }
  void operator () ( this_type& parent_ ) { bind(parent_); }
};

template <typename T>
class hscd_port_storage_out
  : protected hscd_port_base<hscd_root_out_if<T> > {
public:
  typedef T    data_type;
private:
  std::vector<T> s;
protected:
  void              storageClear() { s.clear(); }
  size_t            storageSize() { return s.size(); }
  const data_type  *storageElement( size_t n ) { assert(n < storageSize() ); return &s[n]; }
  
  hscd_port_storage_out( const char *n )
    : hscd_port_base<hscd_root_out_if<data_type> >(n) {}
public:
  data_type &operator []( size_t n ) {
    if ( n >= storageSize() )
      s.resize(n+1);
    return s[n];
  };
};

class hscd_port_storage_out<void>
  : protected hscd_port_base<hscd_root_out_if<void> > {
public:
  typedef void data_type;
private:
protected:
  void              storageClear() {}
  size_t            storageSize() { return UINT_MAX; }
  const data_type  *storageElement( size_t n ) { return NULL; }

  hscd_port_storage_out( const char *n )
    : hscd_port_base<hscd_root_out_if<data_type> >(n) {}
public:
};

template<typename T>
class hscd_port_out
  : public hscd_root_port,
    public hscd_port_storage_out<T> {
public:
  typedef T                   data_type;
  typedef hscd_port_out<T>    this_type;
  typedef hscd_root_out_if<T> iface_type;
private:
//  void setCommittedCount( size_t _committed ) {
//    assert( storageSize() >= _committed );
//    return hscd_root_port::setCommittedCount(_committed);
//  }
  void add_interface( sc_interface *i ) {
    push_interface(i); (*this)->initPortIf( this );
  }
public:
  const T *transferOut( void ) { return storageElement(incrDoneCount()); }
//public:
  hscd_port_out()
    : hscd_port_storage_out<T>( sc_gen_unique_name( "hscd_port_out" )  )
    {}
  
  bool isInput() const { return false; }
  
  size_t availableCount() const { return doneCount() + (*this)->committedInCount(); }
  
  class hscd_op_port operator ()( size_t n ) {
    return hscd_op_port(this,n);
  }
  void operator () ( iface_type& interface_ ) { bind(interface_); }
  void operator () ( this_type& parent_ ) { bind(parent_); }
};

#endif // _INCLUDED_HSCD_POPT_HPP
