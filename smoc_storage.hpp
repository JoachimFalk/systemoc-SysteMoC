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
  
  T& get() {
    assert(valid);
    return *ptr();
  }

  const T& get() const {
    assert(valid);
    return *ptr();
  }

  operator T&() {
    assert(valid);
    return *ptr();
  }

  operator const T&() const {
    assert(valid);
    return *ptr();
  }
  
  void put(const T &t) {
    if(valid) {
      *ptr() = t;
    } else {
      new(mem) T(t);
      valid = true;
    }
  }
  
  ~smoc_storage() {
    if(valid) {
      ptr()->~T();
      valid = false;
    }
  }
};

template<class T>
class smoc_write_only_storage
{
public:
  typedef smoc_write_only_storage<T> this_type;
  
private:
  smoc_storage<T> &s;

public:
  smoc_write_only_storage(smoc_storage<T> &_s) :
    s(_s)
  {}
  
  void operator=(const T& t) {
    s.put(t);
  }  
};


template<class T>
struct smoc_storage_in
{
  typedef const smoc_storage<T> storage_type;
  typedef const T &		return_type;
};

template<class T>
struct smoc_storage_out
{
  typedef smoc_storage<T>	     storage_type;
  typedef smoc_write_only_storage<T> return_type;
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

#endif // SMOC_STORAGE_HPP
