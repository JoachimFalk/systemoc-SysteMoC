// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_FIFO_HPP
#define _INCLUDED_HSCD_FIFO_HPP

#include <hscd_root_if.hpp>
#include <systemc.h>

// #include <iostream>

class hscd_fifo_base {
public:
  typedef hscd_fifo_base  this_type;
protected:
  size_t const fsize;
  size_t       rindex;
  size_t       windex;
  
  size_t rpp() { return rindex == fsize-1 ? (rindex=0,fsize-1) : rindex++; }
  size_t wpp() { return windex == fsize-1 ? (windex=0,fsize-1) : windex++; }
  
  size_t usedStorage() {
    size_t used = windex - rindex;
    
    if ( used > fsize )
      used += fsize;
    return used;
  }
  
  size_t unusedStorage() {
    size_t unused = rindex - windex - 1;
    
    if ( unused > fsize )
      unused += fsize;
    return unused;
  }
public:
  // constructors
  hscd_fifo_base( size_t n )
    : fsize(n+1), rindex(0), windex(0) {}
};

template <typename T, size_t N>
class hscd_fifo_storage
  : public hscd_fifo_base {
public:
  typedef T                      data_type;
  typedef hscd_fifo_storage<T,N> this_type;
private:
  data_type storage[N+1];
protected:
  typedef typename hscd_transfer_port<data_type>::hscd_chan_port_if
    				 iface_type;
  
  void transferInData( iface_type in ) {
    assert( in.haveRequest() );
    for ( ; unusedStorage() && !in.ready(); )
      storage[wpp()] = *in.nextAddr();
  }
  void transferOutData( iface_type out ) {
    assert( out.haveRequest() );
    for ( ; usedStorage() && !out.ready(); )
      *out.nextAddr() = storage[rpp()];
  }
  void copyData( iface_type in, iface_type out ) {
    assert( in.haveRequest() && out.haveRequest() );
    while ( !in.ready() && !out.ready() )
      *out.nextAddr() = *in.nextAddr();
  }
  
  hscd_fifo_storage()
    : hscd_fifo_base(N) {}
};

template <size_t N>
class hscd_fifo_storage<void, N>
  : public hscd_fifo_base {
public:
  typedef void                      data_type;
  typedef hscd_fifo_storage<void,N> this_type;
protected:
  typedef typename hscd_transfer_port<void>::hscd_chan_port_if
    				 iface_type;
  
  void transferInData( iface_type in ) {
    assert( in.haveRequest() );
    for ( ; unusedStorage() && !in.ready(); )
      in.nextAddr();
  }
  void transferOutData( iface_type out ) {
    assert( out.haveRequest() );
    for ( ; usedStorage() && !out.ready(); )
      out.nextAddr();
  }
  void copyData( iface_type in, iface_type out ) {
    assert( in.haveRequest() && out.haveRequest() );
    while ( !in.ready() && !out.ready() ) {
      out.nextAddr(); in.nextAddr();
    }
  }
  
  hscd_fifo_storage()
    : hscd_fifo_base(N) {}
};

template <typename T, size_t N = 1>
class hscd_fifo
  : public hscd_fifo_storage<T,N>,
    public hscd_root_in_if<T>,
    public hscd_root_out_if<T>,
    public sc_prim_channel {
public:
  typedef T               data_type;
  typedef hscd_fifo<T,N>  this_type;
protected:
  typedef typename  hscd_fifo_storage<T,N>::iface_type iface_type;
  
  iface_type in;
  iface_type out;
public:
  // constructors
  hscd_fifo()
    : sc_prim_channel( sc_gen_unique_name( "hscd_fifo" ) ) {}
  
  explicit hscd_fifo( const char* name_ )
    : sc_prim_channel( name_ ) {}
  
  virtual ~hscd_fifo() {}
  
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
  }
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }
  
protected:
  
  // virtual void update();
  
private:
  // disabled
  hscd_fifo( const this_type & );
  
  // disabled
  const sc_event& default_event() const { return hscd_default_event_abort(); }
};

template <typename T, size_t N>
const char* const hscd_fifo<T,N>::kind_string = "hscd_fifo";

#endif // _INCLUDED_HSCD_FIFO_HPP
