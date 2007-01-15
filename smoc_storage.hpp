#ifndef SMOC_STORAGE_HPP
#define SMOC_STORAGE_HPP

#include <cassert>
#include <new>

template<class T>
class smoc_storage
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
  smoc_storage() : valid(false) {
  }

  const T& get() const {
    assert(valid);
    return *ptr();
  }

  operator const T&() const
    { return get(); }

  void put(const T &t) {
    if(valid) {
      *ptr() = t;
    } else {
      new(mem) T(t);
      valid = true;
    }
  }

  void operator=(const T& t)
    { put(t); }  

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
  typedef smoc_storage_wom<T> this_type;
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
  typedef const void storage_type;
  typedef const void return_type;
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
