// vim: set sw=2 ts=8:

#ifndef _INCLUDED_HSCD_RENDEZVOUS_HPP
#define _INCLUDED_HSCD_RENDEZVOUS_HPP

#include <hscd_rendezvous.hpp>

template <typename T>
class hscd_rendezvous
  : public hscd_fifo<T,0> {
public:
  typedef T                     data_type;
  typedef hscd_rendezvous<T>    this_type;
public:
  // constructors
  hscd_rendezvous()
    : hscd_fifo<T,0>( sc_gen_unique_name( "hscd_rendezvous" ) ) {}
  
  explicit hscd_rendezvous( const char* name_ )
    : hscd_fifo<T,0>( name_ ) {}
  
  virtual ~hscd_rendezvous() {}
  
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }
protected:
  
  // virtual void update();
  
private:
  // disabled
  hscd_rendezvous( const this_type & );
};

template <typename T>
const char* const hscd_rendezvous<T>::kind_string = "hscd_rendezvous";

#endif // _INCLUDED_HSCD_RENDEZVOUS_HPP
