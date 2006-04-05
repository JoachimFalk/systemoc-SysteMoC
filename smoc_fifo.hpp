// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */

#ifndef _INCLUDED_SMOC_FIFO_HPP
#define _INCLUDED_SMOC_FIFO_HPP

#include <smoc_chan_if.hpp>
#include <smoc_storage.hpp>

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
  
  void rpp(size_t n) {
    if ( rindex + n >= fsize )
      rindex = rindex + n - fsize;
    else
      rindex = rindex + n;
  }
  void wpp(size_t n) {
    if ( windex + n >= fsize )
      windex = windex + n - fsize;
    else
      windex = windex + n;
  }
  
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

  void channelAttributes(smoc_modes::PGWriter &pgw) const {
    pgw << "<attribute type=\"size\" value=\"" << fsize << "\"/>" << std::endl;
  }

  virtual
  void channelContents(smoc_modes::PGWriter &pgw) const = 0;

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
  typedef smoc_storage<data_type>	     storage_type;
  
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
  storage_type *storage;
protected:
  smoc_fifo_storage( const chan_init &i ) :
    smoc_chan_nonconflicting_if<smoc_fifo_kind, T>(i),
    storage(new storage_type[this->fsize])
  {
    assert(this->fsize > i.marking.size());
    for(size_t j = 0; j < i.marking.size(); ++j) {
      storage[j].put(i.marking[j]);
    }
    this->windex = i.marking.size();
  }
  
  storage_type *getStorage() const { return storage; }
  
  void channelContents(smoc_modes::PGWriter &pgw) const {
    for ( size_t n = 0; n < this->usedStorage(); ++n )
      pgw << "<token value=\"" << storage[n].get() << "\"/>" << std::endl;
  }

  ~smoc_fifo_storage() { delete[] storage; }
};

template <>
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
  smoc_fifo_storage( const chan_init &i )
    : smoc_chan_nonconflicting_if<smoc_fifo_kind, void>(i) {
    assert( this->fsize > i.marking );
    this->windex = i.marking;
  }
  
  void *getStorage() const { return NULL; }

  void channelContents(smoc_modes::PGWriter &pgw) const {
    for ( size_t n = 0; n < this->usedStorage(); ++n )
      pgw << "<token value=\"bot\"/>" << std::endl;
  }
};

template <typename T>
class smoc_fifo_type
  : public smoc_fifo_storage<T> {
public:
  typedef T						      data_type;
  typedef smoc_fifo_type<data_type>			      this_type;
  typedef typename this_type::iface_in_type		      iface_in_type;
  typedef typename this_type::iface_out_type		      iface_out_type;
  
  typedef typename smoc_storage_in<data_type>::storage_type   storage_in_type;
  typedef typename smoc_storage_in<data_type>::return_type    return_in_type;
  typedef smoc_ring_access<storage_in_type, return_in_type>   ring_in_type;
  
  typedef typename smoc_storage_out<data_type>::storage_type  storage_out_type;
  typedef typename smoc_storage_out<data_type>::return_type   return_out_type;
  typedef smoc_ring_access<storage_out_type, return_out_type> ring_out_type;
protected:
//  iface_in_type  *in;
//  iface_out_type *out;
  
  ring_in_type commSetupIn(size_t req) {
    assert( req <= this->usedStorage() );
    return ring_in_type(
      this->getStorage(), this->fsize, this->rindex, req);
  }
  
  void commExecIn(const ring_in_type &r){
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecIn(r.getLimit(), this->name());
#endif
    rpp(r.getLimit()); this->read_event.notify(); 
  }
  
  ring_out_type commSetupOut(size_t req) {
    assert( req <= this->unusedStorage() );
    return ring_out_type(
      this->getStorage(), this->fsize, this->windex, req);
  }
  
  void commExecOut(const ring_out_type &r){
#ifdef SYSTEMOC_TRACE
    TraceLog.traceCommExecOut(r.getLimit(), this->name());
#endif
    wpp(r.getLimit()); this->write_event.notify();
  }
public:
  // constructors
  smoc_fifo_type( const typename smoc_fifo_storage<T>::chan_init &i )
    : smoc_fifo_storage<T>(i) {}
  
  size_t committedOutCount() const {
    return this->usedStorage();// + (portOutIf->committedCount() - portOutIf->doneCount());
  }
  size_t committedInCount() const {
    return this->unusedStorage();// + (portInIf->committedCount() - portInIf->doneCount());
  }
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
