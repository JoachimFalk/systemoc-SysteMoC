// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_FIFO_HPP
#define _INCLUDED_HSCD_FIFO_HPP

#include <hscd_root_if.hpp>
#include <systemc.h>

// #include <iostream>

class hscd_fifo {
  friend class chan_kind;
  
// data types
public:
  class chan_kind
    : public sc_prim_channel {
  public:
    typedef chan_kind  this_type;
  
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
    
    // constructors
    chan_kind( const hscd_fifo &i )
      : sc_prim_channel( i.name != NULL
          ? i.name
          : sc_gen_unique_name( "hscd_fifo" ) ),
        fsize(i.n+1), rindex(0), windex(0) {}
  private:
    static const char* const kind_string;
    
    virtual const char* kind() const {
      return kind_string;
    }
    
    // disabled
    chan_kind( const this_type & );
  };

protected:
  template <typename T>
  class chan_storage
    : public chan_kind {
  public:
    typedef T                      data_type;
    typedef chan_storage<T>   this_type;
  private:
    data_type *storage;
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
    
    chan_storage(const hscd_fifo &i)
      : chan_kind(i), storage(new data_type[fsize]) {}
    
    ~chan_storage() { delete storage; }
  };

  class chan_storage<void>
    : public chan_kind {
  public:
    typedef void                      data_type;
    typedef chan_storage<void>   this_type;
  protected:
    typedef hscd_transfer_port<void>::hscd_chan_port_if iface_type;
    
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
    
    chan_storage(const hscd_fifo &i)
      : chan_kind(i) {}
  };
public:
  template <typename T>
  class chan_type
    : public chan_storage<T>,
      public hscd_root_in_if<T>,
      public hscd_root_out_if<T> {
  public:
    typedef T               data_type;
    typedef chan_type<T>    this_type;
  protected:
    typedef typename  chan_storage<T>::iface_type iface_type;
    
    iface_type in;
    iface_type out;
  public:
    // constructors
    chan_type( const hscd_fifo i = hscd_fifo() )
      : chan_storage<T>(i) {}
    
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
  private:
    // disabled
    const sc_event& default_event() const { return hscd_default_event_abort(); }
  };
  
// member variables
public:
  const char   *name;
  const size_t  n;
  
// constructors, destructors and methods
public:
  hscd_fifo( size_t n = 1 )
    : name(NULL), n(n) {}
  explicit hscd_fifo( const char *name, size_t n = 1)
    : name(name), n(n) {}
};

#endif // _INCLUDED_HSCD_FIFO_HPP
