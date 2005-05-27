// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_RENDEZVOUS_HPP
#define _INCLUDED_HSCD_RENDEZVOUS_HPP

#include <hscd_chan_if.hpp>
#include <systemc.h>

class hscd_rendezvous_kind
  : public hscd_root_chan {
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
    : hscd_root_chan(
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
  : public hscd_chan_nonconflicting_if<hscd_rendezvous_kind, T> {
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
  hscd_rendezvous_type( const hscd_rendezvous_kind::chan_init &i )
    : hscd_chan_nonconflicting_if<hscd_rendezvous_kind, T>(i) {}
  
  size_t committedOutCount() const
    { return (portOutIf->committedCount() - portOutIf->doneCount()); }
//  size_t maxCommittableOutCount() const
//    { return (portOutIf->maxCommittableCount() - portOutIf->doneCount()); }
  size_t committedInCount() const
    { return (portInIf->committedCount() - portInIf->doneCount()); }
//  size_t maxCommittableInCount() const
//    { return (portInIf->maxCommittableCount() - portInIf->doneCount()); }
  void transfer(iface_in_type *_i) { copyData(portOutIf,_i); }
  void transfer(iface_out_type *_o) { copyData(_o,portInIf); }
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
