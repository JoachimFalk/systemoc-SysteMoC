// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_RENDEZVOUS_HPP
#define _INCLUDED_HSCD_RENDEZVOUS_HPP

#include <hscd_root_if.hpp>
#include <systemc.h>

class hscd_rendezvous_kind
  : public sc_prim_channel {
public:
  typedef hscd_rendezvous_kind  this_type;

  class chan_init {
    friend class hscd_rendezvous_kind;
    private:
      const char *name;
    protected:
      chan_init( const char *name )
        : name(name) {}
  };

protected:
  // constructors
  hscd_rendezvous_kind( const chan_init &i )
    : sc_prim_channel(
        i.name != NULL ? i.name : sc_gen_unique_name( "hscd_rendezvous" ) ) {}
private:
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }
  
  // disabled
  hscd_rendezvous_kind( const this_type & );
  
};

template <typename T>
class hscd_rendezvous_copy
  : public hscd_rendezvous_kind {
public:
  typedef T                      data_type;
  typedef hscd_rendezvous_copy<T>           this_type;
protected:
  typedef typename hscd_transfer_port<data_type>::hscd_chan_port_if
                                 iface_type;
  
  void copyData( iface_type in, iface_type out ) {
    assert( in.haveRequest() && out.haveRequest() );
    while ( !in.ready() && !out.ready() )
      *out.nextAddr() = *in.nextAddr();
  }
  
  hscd_rendezvous_copy( const chan_init &i )
    : hscd_rendezvous_kind(i) {}
};

class hscd_rendezvous_copy<void>
  : public hscd_rendezvous_kind {
public:
  typedef void                      data_type;
  typedef hscd_rendezvous_copy<void>           this_type;
protected:
  typedef hscd_transfer_port<void>::hscd_chan_port_if
                                    iface_type;
  
  void copyData( iface_type in, iface_type out ) {
    assert( in.haveRequest() && out.haveRequest() );
    while ( !in.ready() && !out.ready() ) {
      out.nextAddr(); in.nextAddr();
    }
  }
  
  hscd_rendezvous_copy( const chan_init &i )
    : hscd_rendezvous_kind(i) {}
};

template <typename T>
class hscd_rendezvous_type
  : public hscd_rendezvous_copy<T>,
    public hscd_root_in_if<T>,
    public hscd_root_out_if<T> {
public:
  typedef T               data_type;
  typedef hscd_rendezvous_type<T>    this_type;
protected:
  typedef typename  hscd_rendezvous_copy<T>::iface_type iface_type;
  
  iface_type in;
  iface_type out;
public:
  // constructors
  hscd_rendezvous_type( const typename hscd_rendezvous_type<T>::chan_init &i )
    : hscd_rendezvous_copy<T>(i) {}
  
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

template <typename T>
class hscd_rendezvous
  : public hscd_rendezvous_kind::chan_init {
public:
  typedef T                         data_type;
  typedef hscd_rendezvous<T>        this_type;
  typedef hscd_rendezvous_type<T>   chan_type;
  
  hscd_rendezvous()
    : hscd_rendezvous_kind::chan_init(NULL) {}
  explicit hscd_rendezvous( const char *name )
    : hscd_rendezvous_kind::chan_init(name) {}
};


#endif // _INCLUDED_HSCD_RENDEZVOUS_HPP
