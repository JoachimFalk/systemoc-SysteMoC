// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_RENDEZVOUS_HPP
#define _INCLUDED_HSCD_RENDEZVOUS_HPP

#include <hscd_root_if.hpp>
#include <systemc.h>

const sc_event& hscd_default_event_abort();

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
  this_type& operator = ( const this_type & );
};

template <typename T>
class hscd_rendezvous_type
  : public hscd_rendezvous_kind,
    public hscd_root_in_if<T>,
    public hscd_root_out_if<T> {
public:
  typedef T                                  data_type;
  typedef hscd_rendezvous_type<data_type>    this_type;
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
  hscd_rendezvous_type( const chan_init &i )
    : hscd_rendezvous_kind(i) {}
  
  size_t committedOutCount() const
    { return portOutIf->committedCount(); }
  size_t committedInCount() const
    { return portInIf->committedCount(); }
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
