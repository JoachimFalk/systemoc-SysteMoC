// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_POPT_HPP
#define _INCLUDED_SMOC_POPT_HPP

#include <expr.hpp>

#include <smoc_root_port.hpp>
#include <smoc_chan_if.hpp>

#include <systemc.h>
#include <vector>

template <typename T> class smoc_port_in;
template <typename T> class smoc_port_out;

/****************************************************************************
 * DExprToken is a placeholder for a token in the expression.
 */

class DNodeToken: public DNodeTerminal {
public:
  DNodeToken(const exInfo &i)
    : DNodeTerminal(i) {}
};

template<typename T>
class DExprToken {
public:
  typedef T value_type;
private:
  smoc_port_in<T> &p;
  size_t           pos;
public:
  explicit DExprToken(smoc_port_in<T> &p, size_t pos)
    : p(p), pos(pos) {}
  
  PDNodeBase getNodeType() const
    { return PDNodeBase(new DNodeToken(this)); }
  
  value_type value() const
    { return T(); }
};

template<class T>
struct DExpr<DExprToken<T> >: public DExprBase<DExprToken<T> > {
  DExpr(smoc_port_in<T> &p, size_t pos)
    : DExprBase<DExprToken<T> >(DExprToken<T>(p,pos)) {}
};

// Make a convenient typedef for the token type.
template<typename T>
struct DToken {
  typedef DExpr<DExprToken<T> > type;
};

template <typename T>
typename DToken<T>::type token(smoc_port_in<T> &p, size_t pos)
  { return typename DToken<T>::type(p,pos); }

/****************************************************************************/

template <typename T>
class smoc_port_base
  : public smoc_root_port {
public:
  typedef T                   iface_type;
  typedef smoc_port_base<T>   this_type;
private:
  typedef smoc_root_port      base_type;
  
  iface_type  *interface;
  
  const char *if_typename() const { return typeid(iface_type).name(); }

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
protected:
  smoc_port_base( const char* name_ )
    : base_type( name_ ), interface( NULL ) {}
  
  void push_interface( sc_interface *_i ) {
    assert( interface == NULL );
    interface = dynamic_cast<iface_type *>(_i);
    assert( interface != NULL );
  }

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
class smoc_port_storage_in
  : public smoc_port_base<smoc_chan_in_if<T> > {
public:
  typedef T    data_type;
private:
  std::vector<T> s;
protected:
  void storageClear() { s.clear(); }
  void storagePushBack( const data_type *in ) { s.push_back(*in); }
  
  smoc_port_storage_in( const char *n )
    : smoc_port_base<smoc_chan_in_if<data_type> >(n) {}
public:
  data_type &operator []( size_t n )
    { assert( n < s.size() ); return s[n]; }
};

class smoc_port_storage_in<void>
  : public smoc_port_base<smoc_chan_in_if<void> > {
public:
  typedef void data_type;
private:
protected:
  void storageClear() {}
  void storagePushBack( const data_type *in ) { assert(in == NULL); }
  
  smoc_port_storage_in( const char *n )
    : smoc_port_base<smoc_chan_in_if<data_type> >(n) {}
public:
};

template <typename T>
class smoc_port_in
  : public smoc_port_storage_in<T> {
public:
  typedef T                               data_type;
  typedef smoc_port_in<T>                 this_type;
  typedef typename this_type::iface_type  iface_type;
  typedef smoc_port_storage_in<T>         base_type;
private:
//  void setCommittedCount( size_t _committed ) {
//    return smoc_root_port::setCommittedCount(_committed);
//  }
  
  void reset() {
    storageClear(); return base_type::reset();
  }
  
  void add_interface( sc_interface *i ) {
    push_interface(i); (*this)->addPortIf( this );
  }
  
  void transfer() { (*this)->transfer(this); }
public:
  void transferIn( const T *in ) { storagePushBack(in); incrDoneCount(); }
//public:
  smoc_port_in()
    : smoc_port_storage_in<T>( sc_gen_unique_name( "smoc_port_in" )  )
    {}
  
  bool isInput() const { return true; }
  
  size_t availableCount()    const { return doneCount() + (*this)->committedOutCount(); }
//  size_t maxAvailableCount() const { return doneCount() + (*this)->maxCommittableOutCount(); }
  
  typename DToken<T>::type getValueAt(size_t n)
    { return token(*this,n); }
  
  typename DCommReq::type getAvailableTokens()
    { return commreq(*this); }
  DExpr<DExprBinOp<DExprCommReq,DExprLiteral<size_t>,DOpGe<size_t,size_t> > >
  operator ()( size_t n )
    { return getAvailableTokens() >= n; }
  
  void operator () ( iface_type& interface_ )
    { bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};

template <typename T>
class smoc_port_storage_out
  : public smoc_port_base<smoc_chan_out_if<T> > {
public:
  typedef T    data_type;
private:
  std::vector<T> s;
protected:
  void              storageClear() { s.clear(); }
  size_t            storageSize() { return s.size(); }
  const data_type  *storageElement( size_t n ) { assert(n < storageSize() ); return &s[n]; }
  
  smoc_port_storage_out( const char *n )
    : smoc_port_base<smoc_chan_out_if<data_type> >(n) {}
public:
  data_type &operator []( size_t n ) {
    if ( n >= storageSize() )
      s.resize(n+1);
    return s[n];
  };
};

class smoc_port_storage_out<void>
  : public smoc_port_base<smoc_chan_out_if<void> > {
public:
  typedef void data_type;
private:
protected:
  void              storageClear() {}
  size_t            storageSize() { return UINT_MAX; }
  const data_type  *storageElement( size_t n ) { return NULL; }

  smoc_port_storage_out( const char *n )
    : smoc_port_base<smoc_chan_out_if<data_type> >(n) {}
public:
};

template<typename T>
class smoc_port_out
  : public smoc_port_storage_out<T> {
public:
  typedef T                               data_type;
  typedef smoc_port_out<T>                this_type;
  typedef typename this_type::iface_type  iface_type;
  typedef smoc_port_storage_out<T>        base_type;
private:
//  void setCommittedCount( size_t _committed ) {
//    assert( storageSize() >= _committed );
//    return smoc_root_port::setCommittedCount(_committed);
//  }
  void add_interface( sc_interface *i ) {
    push_interface(i); (*this)->addPortIf( this );
  }

  void transfer() { (*this)->transfer(this); }
public:
  const T *transferOut( void ) { return storageElement(incrDoneCount()); }
//public:
  smoc_port_out()
    : smoc_port_storage_out<T>( sc_gen_unique_name( "smoc_port_out" )  )
    {}
  
  bool isInput() const { return false; }
  
  size_t availableCount()    const { return doneCount() + (*this)->committedInCount(); }
//  size_t maxAvailableCount() const { return doneCount() + (*this)->maxCommittableInCount(); }
  
  DCommReq::type getAvailableSpace()
    { return commreq(*this); }
  DExpr<DExprBinOp<DExprCommReq,DExprLiteral<size_t>,DOpGe<size_t,size_t> > >
  operator ()( size_t n )
    { return getAvailableSpace() >= n; }
  
  void operator () ( iface_type& interface_ )
    { bind(interface_); }
  void operator () ( this_type& parent_ )
    { bind(parent_); }
};

#endif // _INCLUDED_SMOC_POPT_HPP
