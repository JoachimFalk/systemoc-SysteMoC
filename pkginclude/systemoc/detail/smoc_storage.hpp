// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef SMOC_STORAGE_HPP
#define SMOC_STORAGE_HPP

#include <cassert>
#include <new>

#include <boost/type_traits.hpp> 

#include <CoSupport/SystemC/ChannelModificationListener.hpp> 

#include <systemoc/smoc_config.h>

template<class T,
         bool is_subclass =
         boost::is_base_of<CoSupport::SystemC::ChannelModificationListener, T>::value>
class smoc_modification_listener{
public:
  void setChannelID( std::string sourceActor,
                     CoSupport::SystemC::ChannelId id,
                     std::string name ){}
protected:
  void fireModified( const T &t ) const {}
};

template<class T>
class smoc_modification_listener <T, true>{
public:
  void setChannelID( std::string sourceActor,
                     CoSupport::SystemC::ChannelId id,
                     std::string name ){
    //FIXME:
    T t; t.registerChannel(sourceActor, id, name);

    channelId = id;
    //cerr << "2  setChannelID " << sourceActor << " " << name << " "
    //     << id << endl;
    
  }
protected:
  void fireModified( const T &t ) const {
    //cerr << "2 fireModified(...) " << endl;
    t.modified(channelId);
  }
private:
  CoSupport::SystemC::ChannelId channelId;
};

template<class T>
class smoc_storage
  : public smoc_modification_listener<T>
{
private:
  char mem[sizeof(T)];
  bool valid;
private:
  T* ptr()
  { return reinterpret_cast<T*>(mem); }

  const T* ptr() const
  { return reinterpret_cast<const T*>(mem); }
public:
  smoc_storage() : valid(false) {}

  smoc_storage(const T& t) : valid(true) { new(mem) T(t); }

  T &get() {
    assert(valid);
    T &t = *ptr();
    // delayed read access (VPC) may conflict with channel (in-)validation
    this->fireModified( t );
    return t;
  }

  operator T &()
    { return get(); }

  const T& get() const {
    assert(valid);
    const T &t = *ptr();
    // delayed read access (VPC) may conflict with channel (in-)validation
    this->fireModified( t );
    return t;
  }

  operator const T&() const
    { return get(); }

  void put(const T &t) {
    this->fireModified( t );
    if(valid) {
      *ptr() = t;
    } else {
      new(mem) T(t);
      valid = true;
    }
  }

  //FIXME(MS): allow "const" access for non-strict
  //void operator=(const T& t) const {
  void operator=(const T& t)
    { put(t); }  

  const bool isValid() const{
    return valid;
  }

  // invalidate data (e.g., to avoid reread of commited data)
  void invalidate()
  {
    if(valid) {
      ptr()->~T();
      valid = false;
    }
  }

  // reset storage
  // used in SR (signals are reseted if fixed point is reached)
  //
  // (FIXME: may also be used for destructive reading)
  void reset(){
    valid = false;
  }
  
  ~smoc_storage() {
    if(valid) {
      ptr()->~T();
      valid = false;
    }
  }
};

/// smoc storage with read only memory interface
// typedef const T & smoc_storage_rom<T>

/// smoc storage with write only memory interface
template<class T>
class smoc_storage_wom
{
private:
  typedef smoc_storage_wom<T> this_type;
private:
  smoc_storage<T> &s;
public:
  smoc_storage_wom(smoc_storage<T> &_s)
    : s(_s) {}

  void operator=(const T& t)
    { s.put(t); }  
};

/// smoc storage with read write memory interface
template<class T>
class smoc_storage_rw
{
private:
  typedef smoc_storage_rw<T> this_type;
private:
  smoc_storage<T> &s;
public:
  smoc_storage_rw(smoc_storage<T> &_s)
    : s(_s) {}
 
  operator const T&() const
    { return s.get(); }

  void operator=(const T& t)
    { s.put(t); }  
};

namespace CoSupport {
  // provide isType for smoc_storage_rw
  template <typename T, typename X>
  static inline
  bool isType( const smoc_storage_rw<X> &of )
   { return isType<T>(static_cast<const X &>(of)); }
};

template<class T>
struct smoc_storage_in
{
  typedef const smoc_storage<T>  storage_type;
  typedef const T               &return_type;
};

template<class T>
struct smoc_storage_out
{
  typedef smoc_storage<T>        storage_type;
  typedef smoc_storage_wom<T>    return_type;
};

template<class T>
struct smoc_storage_inout
{
  typedef smoc_storage<T>        storage_type;
  typedef smoc_storage_rw<T>     return_type;
};

template<>
struct smoc_storage_in<void>
{
  typedef void storage_type;
  typedef void return_type;
};

template<>
struct smoc_storage_out<void>
{
  typedef void storage_type;
  typedef void return_type;
};

template<>
struct smoc_storage_inout<void>
{
  typedef void storage_type;
  typedef void return_type;
};

#endif // SMOC_STORAGE_HPP
