// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_FIFO_HPP
#define _INCLUDED_HSCD_FIFO_HPP

#include <hscd_chan_if.hpp>

#include <systemc.h>
#include <vector>

// #include <iostream>

class hscd_fifo_kind
  : public hscd_root_chan {
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
    : hscd_root_chan(
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
  : public hscd_chan_nonconflicting_if<hscd_fifo_kind, T> {
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
    : hscd_chan_nonconflicting_if<hscd_fifo_kind, T>(i), storage(new data_type[fsize]) {
    assert( fsize > i.marking.size() );
    memcpy( storage, &i.marking[0], i.marking.size()*sizeof(T) );
    windex = i.marking.size();
  }
  
  ~hscd_fifo_storage() { delete storage; }
};

class hscd_fifo_storage<void>
  : public hscd_chan_nonconflicting_if<hscd_fifo_kind, void> {
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
    : hscd_chan_nonconflicting_if<hscd_fifo_kind, void>(i) {
    assert( fsize > i.marking );
    windex = i.marking;
  }
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

#endif // _INCLUDED_HSCD_FIFO_HPP
