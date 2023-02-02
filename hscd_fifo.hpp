// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2006 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_HSCD_FIFO_HPP
#define _INCLUDED_HSCD_FIFO_HPP

#include <smoc_fifo.hpp>

// #include <iostream>

/*
class hscd_fifo_kind
  : public sc_prim_channel {
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
  hscd_fifo_kind( const chan_init &i )
    : sc_prim_channel(
        i.name != NULL ? i.name : sc_gen_unique_name( "hscd_fifo" ) ),
      fsize(i.n+1), rindex(0), windex(0) {}
private:
  static const char* const kind_string;
  
  virtual const char* kind() const {
    return kind_string;
  }
  
  // disabled
  hscd_fifo_kind( const this_type & );
};

template <typename T>
class hscd_fifo_storage
  : public hscd_fifo_kind {
public:
  typedef T                      data_type;
  typedef hscd_fifo_storage<T>   this_type;
  
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
 
  hscd_fifo_storage( const chan_init &i )
    : hscd_fifo_kind(i), storage(new data_type[fsize]) {
    assert( fsize > i.marking.size() );
    memcpy( storage, &i.marking[0], i.marking.size()*sizeof(T) );
    windex = i.marking.size();
  }
  
  ~hscd_fifo_storage() { delete storage; }
};

class hscd_fifo_storage<void>
  : public hscd_fifo_kind {
public:
  typedef void                      data_type;
  typedef hscd_fifo_storage<void>   this_type;
  
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
  
  hscd_fifo_storage( const chan_init &i )
    : hscd_fifo_kind(i) {
    assert( fsize > i.marking );
    windex = i.marking;
  }
};

template <typename T>
class hscd_fifo_type
  : public hscd_fifo_storage<T>,
    public hscd_root_in_if<T>,
    public hscd_root_out_if<T> {
public:
  typedef T                   data_type;
  typedef hscd_fifo_type<T>   this_type;
protected:
  typedef typename  hscd_fifo_storage<T>::iface_type iface_type;
  
  iface_type in;
  iface_type out;
public:
  // constructors
  hscd_fifo_type( const typename hscd_fifo_storage<T>::chan_init &i )
    : hscd_fifo_storage<T>(i) {}
  
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

*/

template <typename T>
class hscd_fifo
  : public smoc_fifo<T> {
public:
  typedef T                   data_type;
  typedef hscd_fifo<T>        this_type;
  
  hscd_fifo( size_t n = 1 )
    : smoc_fifo<T>(n) {}
  explicit hscd_fifo( const char *name, size_t n = 1)
    : smoc_fifo<T>(name,n) {}
};

#endif // _INCLUDED_HSCD_FIFO_HPP
