// vim: set sw=2 ts=8:

#ifndef _INCLUDED_SMOC_FIFO_HPP
#define _INCLUDED_SMOC_FIFO_HPP

#include <smoc_chan_if.hpp>

#include <systemc.h>
#include <vector>

// #include <iostream>

class smoc_fifo_kind
  : public smoc_root_chan {
public:
  typedef smoc_fifo_kind  this_type;
  
  class chan_init {
    friend class smoc_fifo_kind;
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
  smoc_fifo_kind( const chan_init &i )
    : smoc_root_chan(
        i.name != NULL ? i.name : sc_gen_unique_name( "smoc_fifo" ) ),
      fsize(i.n+1), rindex(0), windex(0) {}
private:
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }
  
  // disabled
  smoc_fifo_kind( const this_type & );
  this_type& operator = ( const this_type & );
};

template <typename T>
class smoc_fifo_storage
  : public smoc_chan_nonconflicting_if<smoc_fifo_kind, T> {
public:
  typedef T                                  data_type;
  typedef smoc_fifo_storage<data_type>       this_type;
  typedef typename this_type::iface_out_type iface_out_type;
  typedef typename this_type::iface_in_type  iface_in_type;
  
  class chan_init
    : public smoc_fifo_kind::chan_init {
    friend class smoc_fifo_storage<T>;
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
      : smoc_fifo_kind::chan_init(name, n) {}
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
  
  smoc_fifo_storage( const chan_init &i )
    : smoc_chan_nonconflicting_if<smoc_fifo_kind, T>(i), storage(new data_type[fsize]) {
    assert( fsize > i.marking.size() );
    memcpy( storage, &i.marking[0], i.marking.size()*sizeof(T) );
    windex = i.marking.size();
  }
  
  ~smoc_fifo_storage() { delete storage; }
};

class smoc_fifo_storage<void>
  : public smoc_chan_nonconflicting_if<smoc_fifo_kind, void> {
public:
  typedef void                               data_type;
  typedef smoc_fifo_storage<data_type>       this_type;
  
  class chan_init
    : public smoc_fifo_kind::chan_init {
    friend class smoc_fifo_storage<void>;
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
      : smoc_fifo_kind::chan_init(name, n),
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
  
  smoc_fifo_storage( const chan_init &i )
    : smoc_chan_nonconflicting_if<smoc_fifo_kind, void>(i) {
    assert( fsize > i.marking );
    windex = i.marking;
  }
};

template <typename T>
class smoc_fifo_type
  : public smoc_fifo_storage<T> {
public:
  typedef T                                  data_type;
  typedef smoc_fifo_type<data_type>          this_type;
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
  smoc_fifo_type( const typename smoc_fifo_storage<T>::chan_init &i )
    : smoc_fifo_storage<T>(i) {}
  
  size_t committedOutCount() const {
    return usedStorage();// + (portOutIf->committedCount() - portOutIf->doneCount());
  }
  size_t maxCommittedOutCount() const {
    return usedStorage();// + (portOutIf->maxCommittedCount() - portOutIf->doneCount());
  }
  size_t committedInCount() const {
    return unusedStorage();// + (portInIf->committedCount() - portInIf->doneCount());
  }
  size_t maxCommittedInCount() const {
    return unusedStorage();// + (portInIf->maxCommittedCount() - portInIf->doneCount());
  }
  
//  size_t maxCommittableOutCount() const
//    { return usedStorage(); }
//  size_t maxCommittableInCount() const
//    { return unusedStorage(); }
  
  void transfer(iface_in_type *_i) { transferOutData(_i); }
  void transfer(iface_out_type *_o) { transferInData(_o); }
};

template <typename T>
class smoc_fifo
  : public smoc_fifo_storage<T>::chan_init {
public:
  typedef T                   data_type;
  typedef smoc_fifo<T>        this_type;
  typedef smoc_fifo_type<T>   chan_type;
  
  this_type &operator <<( typename smoc_fifo_storage<T>::chan_init::add_param_ty x ) {
    add(x); return *this;
  }
  
  smoc_fifo( size_t n = 1 )
    : smoc_fifo_storage<T>::chan_init(NULL,n) {}
  explicit smoc_fifo( const char *name, size_t n = 1)
    : smoc_fifo_storage<T>::chan_init(name,n) {}
};

#endif // _INCLUDED_SMOC_FIFO_HPP
