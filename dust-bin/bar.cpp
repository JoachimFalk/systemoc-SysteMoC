
#include <systemc.h>

#include <iostream>

#include <cstdlib>
#include <cstdio>
#include <cassert>
#include <climits>

#include <vector>

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

template <typename T> class hscd_port_in;
template <typename T> class hscd_port_out;

class hscd_op_port {
public:
  typedef hscd_op_port  this_type;
private:
  hscd_root_port *port;
  size_t          tokens;
protected:
  template <typename T> friend class hscd_port_in;
  template <typename T> friend class hscd_port_out;
  
  hscd_op_port( hscd_root_port *port, size_t tokens )
    : port(port), tokens(tokens) {}
public:
  operator hscd_root_port &() { return *port; }
  
  hscd_root_port       *getPort() { return port; }
  const hscd_root_port *getPort() const { return port; }
  
  bool canSatisfy() const { return tokens == port->availableCount(); }
  bool satisfied()  const { return tokens == port->doneCount(); }
  bool isInput()    const { return port->isInput();  }
  bool isOutput()   const { return port->isOutput(); }
};

const sc_event& hscd_default_event_abort();

template <typename T>
class hscd_root_in_if
  : virtual public sc_interface
{
public:
  // typedefs
  typedef T                    data_type;
  typedef hscd_root_in_if<T>   this_type;
  typedef hscd_port_in<T>      iface_in_type;
protected:
  iface_in_type *portInIf;
public:
  // virtual const sc_event& wantEvent() const = 0;
  //  virtual void wantData( iface_type tq ) = 0;
  
  void initPortIf(iface_in_type *_i) { portInIf = _i; }
  virtual size_t committedOutCount() const = 0;
protected:  
  // constructor
  hscd_root_in_if() : portInIf(NULL) {}
private:
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
  // disabled
  hscd_root_in_if( const this_type& );
  this_type &operator = ( const this_type & );
};

template <typename T>
class hscd_root_out_if
  : virtual public sc_interface
{
public:
  // typedefs
  typedef T                     data_type;
  typedef hscd_root_out_if<T>   this_type;
  typedef hscd_port_out<T>      iface_out_type;
protected:
  iface_out_type *portOutIf;
public:
  // virtual const sc_event& provideEvent() const = 0;
  // virtual void provideData( iface_type tq ) = 0;
  
  void initPortIf(iface_out_type *_i) { portOutIf = _i; }
  virtual size_t committedInCount() const = 0;
protected:
  // constructor
  hscd_root_out_if() : portOutIf(NULL) {}
private:
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
  // disabled
  hscd_root_out_if( const this_type& );
  this_type& operator = ( const this_type & );
};

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

class hscd_fifo_kind
  : public sc_prim_channel {
public:
  typedef hscd_fifo_kind  this_type;
  
  class chan_init {
    friend class hscd_fifo_kind;
  private:
    const char *name;
    size_t      n;
  protected:
    chan_init( const char *name, size_t n )
      : name(name), n(n) {}
  };
protected:
  size_t const fsize;
  size_t       rindex;
  size_t       windex;
  
  size_t rpp() { return rindex == fsize-1 ? (rindex=0,fsize-1) : rindex++; }
  size_t wpp() { return windex == fsize-1 ? (windex=0,fsize-1) : windex++; }
  
  size_t usedStorage() const {
    size_t used = windex - rindex;
    
    if ( used > fsize )
      used += fsize;
    return used;
  }
  
  size_t unusedStorage() const {
    size_t unused = rindex - windex - 1;
    
    if ( unused > fsize )
      unused += fsize;
    return unused;
  }
  
  // constructors
  hscd_fifo_kind( const chan_init &i )
    : sc_prim_channel(
        i.name != NULL ? i.name : sc_gen_unique_name( "hscd_fifo" ) ),
      fsize(i.n+1), rindex(0), windex(0) {}
private:
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }
  
  // disabled
  hscd_fifo_kind( const this_type & );
  this_type& operator = ( const this_type & );
};

template <typename T>
class hscd_fifo_storage
  : public hscd_fifo_kind,
    public hscd_root_in_if<T>,
    public hscd_root_out_if<T> {
public:
  typedef T                                  data_type;
  typedef hscd_fifo_storage<data_type>       this_type;
  typedef typename this_type::iface_out_type iface_out_type;
  typedef typename this_type::iface_in_type  iface_in_type;
  
  class chan_init
    : public hscd_fifo_kind::chan_init {
    friend class hscd_fifo_storage<T>;
  private:
    std::vector<T>  marking;
  protected:
    typedef const T add_param_ty;
  public:
    void add( add_param_ty x ) {
      marking.push_back(x);
    }
  protected:
    chan_init( const char *name, size_t n )
      : hscd_fifo_kind::chan_init(name, n) {}
  };
private:
  data_type *storage;
protected:
  void transferInData( iface_out_type *out ) {
    for ( ; unusedStorage() && out->canTransfer(); )
      storage[wpp()] = *out->transferOut();
  }
  void transferOutData( iface_in_type *in ) {
    for ( ; usedStorage() && in->canTransfer(); )
      in->transferIn( &storage[rpp()] );
  }
  
  hscd_fifo_storage( const chan_init &i )
    : hscd_fifo_kind(i), storage(new data_type[fsize]) {
    assert( fsize > i.marking.size() );
    memcpy( storage, &i.marking[0], i.marking.size()*sizeof(T) );
    windex = i.marking.size();
  }
  
  ~hscd_fifo_storage() { delete storage; }
private:
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
};

class hscd_fifo_storage<void>
  : public hscd_fifo_kind,
    public hscd_root_in_if<void>,
    public hscd_root_out_if<void> {
public:
  typedef void                               data_type;
  typedef hscd_fifo_storage<data_type>       this_type;
  
  class chan_init
    : public hscd_fifo_kind::chan_init {
    friend class hscd_fifo_storage<void>;
  private:
    size_t          marking;
  protected:
    typedef size_t  add_param_ty;
  public:
    void add( add_param_ty x ) {
      marking += x;
    }
  protected:
    chan_init( const char *name, size_t n )
      : hscd_fifo_kind::chan_init(name, n),
        marking(0) {}
  };
protected:
  void transferInData( iface_out_type *out ) {
    for ( ; unusedStorage() && out->canTransfer(); )
      out->transferOut();
  }
  void transferOutData( iface_in_type *in ) {
    for ( ; usedStorage() && in->canTransfer(); )
      in->transferIn(NULL);
  }
  
  hscd_fifo_storage( const chan_init &i )
    : hscd_fifo_kind(i) {
    assert( fsize > i.marking );
    windex = i.marking;
  }
private:
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
};

template <typename T>
class hscd_fifo_type
  : public hscd_fifo_storage<T> {
public:
  typedef T                                  data_type;
  typedef hscd_fifo_type<data_type>          this_type;
  typedef typename this_type::iface_in_type  iface_in_type;
  typedef typename this_type::iface_out_type iface_out_type;
protected:
  iface_in_type  *in;
  iface_out_type *out;
  
  void copyData( iface_out_type *out, iface_in_type *in ) {
    while ( in->canTransfer() && out->canTransfer() )
      in->transferIn( out->transferOut() );
  }
public:
  // constructors
  hscd_fifo_type( const typename hscd_fifo_storage<T>::chan_init &i )
    : hscd_fifo_storage<T>(i) {}
  
  size_t committedOutCount() const
    { return usedStorage() + portOutIf->committedCount(); }
  size_t committedInCount() const
    { return unusedStorage() + portInIf->committedCount(); }

/*  
  // interface methods
  virtual void wantData( iface_type tr ) {
    //std::cerr << "call wantData( " << tr << ", " << tr->request_count << ", " << tr->done_count << " );" << std::endl;
    transferOutData(tr);
    if ( in.haveRequest() ) {
      copyData(in,tr);
      transferInData(in);
      in.notify();
    }
    if ( !tr.ready() ) {
      out = tr;
      out.setCancler();
    }
    //std::cerr << "return wantData( " << tr << ", " << tr->request_count << ", " << tr->done_count << " );" << std::endl;
  }
  
  virtual void provideData( iface_type tr ) {
    //std::cerr << "call provideData( " << tr << ", " << tr->request_count << ", " << tr->done_count << " );" << std::endl;
    if ( out.haveRequest() ) {
      assert( usedStorage() == 0 ); // transferOut(out); should not be neccessary
      copyData(tr,out);
      out.notify();
    }
    transferInData(tr);
    if ( !tr.ready() ) {
      in = tr;
      in.setCancler();
    }
    //std::cerr << "return provideData( " << tr << ", " << tr->request_count << ", " << tr->done_count << " );" << std::endl;
  } */
};

template <typename T>
class hscd_fifo
  : public hscd_fifo_storage<T>::chan_init {
public:
  typedef T                   data_type;
  typedef hscd_fifo<T>        this_type;
  typedef hscd_fifo_type<T>   chan_type;
  
  this_type &operator <<( typename hscd_fifo_storage<T>::chan_init::add_param_ty x ) {
    add(x); return *this;
  }
  
  hscd_fifo( size_t n = 1 )
    : hscd_fifo_storage<T>::chan_init(NULL,n) {}
  explicit hscd_fifo( const char *name, size_t n = 1)
    : hscd_fifo_storage<T>::chan_init(name,n) {}
};

SC_MODULE(m_foo) {
public:
  hscd_port_in<int>  x1;
  hscd_port_in<void> x2;
  hscd_port_out<int>  y1;
  hscd_port_out<void> y2;
private:
  void process(void) {
    x1.setCommittedCount(2);
    x2.setCommittedCount(2);
    y1.setCommittedCount(4);
    y2.setCommittedCount(4);
    x1(12);
    if ( 0 )
      y1[0] = x1[0];
    std::cout << "foo" << std::endl;
    std::cout << x1.availableCount() << std::endl;
  }
public:
  SC_CTOR(m_foo) {
    SC_THREAD(process);
  }
};

int sc_main( int argc, char *argv[] ) {
  hscd_fifo_type<int>  f1( hscd_fifo<int>(2) << 1 );
  hscd_fifo_type<void> f2( hscd_fifo<void>(10) << 3 );
  hscd_fifo_type<int>  f3( hscd_fifo<int>(2) << 1 << 2 );
  hscd_fifo_type<void> f4( hscd_fifo<void>(10) << 0 );
  
  m_foo foo1("foo1");
  m_foo foo2("foo2");
  
  foo1.x1(f1);
  foo2.y1(f1);
  foo1.x2(f2);
  foo2.y2(f2);
  foo1.y1(f3);
  foo2.x1(f3);
  foo1.y2(f4);
  foo2.x2(f4);

  sc_start(-1);
  
  return 0;
}


const char* const hscd_fifo_kind::kind_string = "hscd_fifo";

const sc_event& hscd_default_event_abort() {
  assert(0);
}
