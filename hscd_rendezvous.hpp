// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_RENDEZVOUS_HPP
#define _INCLUDED_HSCD_RENDEZVOUS_HPP

#include <hscd_root_if.hpp>
#include <systemc.h>

class hscd_rendezvous {
// data types
public:
  class chan_kind
    : public sc_prim_channel {
  public:
    typedef chan_kind  this_type;
  protected:
    // constructors
    chan_kind( const hscd_rendezvous &i )
      : sc_prim_channel( i.name != NULL
          ? i.name
          : sc_gen_unique_name( "hscd_rendezvous" ) ) {}
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
  class chan_copy
    : public chan_kind {
  public:
    typedef T                      data_type;
    typedef chan_copy<T>           this_type;
  protected:
    typedef typename hscd_transfer_port<data_type>::hscd_chan_port_if
                                   iface_type;
    
    void copyData( iface_type in, iface_type out ) {
      assert( in.haveRequest() && out.haveRequest() );
      while ( !in.ready() && !out.ready() )
        *out.nextAddr() = *in.nextAddr();
    }
    
    chan_copy(const hscd_rendezvous &i)
      : chan_kind(i) {}
  };

  class chan_copy<void>
    : public chan_kind {
  public:
    typedef void                      data_type;
    typedef chan_copy<void>           this_type;
  protected:
    typedef hscd_transfer_port<void>::hscd_chan_port_if
                                      iface_type;
    
    void copyData( iface_type in, iface_type out ) {
      assert( in.haveRequest() && out.haveRequest() );
      while ( !in.ready() && !out.ready() ) {
        out.nextAddr(); in.nextAddr();
      }
    }
    
    chan_copy(const hscd_rendezvous &i)
      : chan_kind(i) {}
  };
public:
  template <typename T>
  class chan_type
    : public chan_copy<T>,
      public hscd_root_in_if<T>,
      public hscd_root_out_if<T> {
  public:
    typedef T               data_type;
    typedef chan_type<T>    this_type;
  protected:
    typedef typename  chan_copy<T>::iface_type iface_type;
    
    iface_type in;
    iface_type out;
  public:
    // constructors
    chan_type( const hscd_rendezvous i = hscd_rendezvous() )
      : chan_copy<T>(i) {}
    
    // interface methods
    virtual void wantData( iface_type tr ) {
      //std::cerr << "call wantData( " << tr << ", " << tr->request_count << ", " << tr->done_count << " );" << std::endl;
      if ( in.haveRequest() ) {
        copyData(in,tr);
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
        copyData(tr,out);
        out.notify();
      }
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
  const char *name;
  
// constructors, destructors and methods
public:
  hscd_rendezvous( const char *name = NULL )
    : name(name) {}
};

#endif // _INCLUDED_HSCD_RENDEZVOUS_HPP
