// vim: set sw=2 ts=8:

#ifndef _INCLUDED_ONEOF_HPP
#define _INCLUDED_ONEOF_HPP

struct void2_st {
};
struct void3_st {
};

template <typename T1, typename T2 = void2_st, typename T3 = void3_st>
class oneof {
  private:
    int valid;
    
    union {
      char e1[sizeof(T1)];
      char e2[sizeof(T2)];
      char e3[sizeof(T3)];
    } mem;
  public:
    oneof(): valid(0) {
    }
    oneof(const T1 &e): valid(1) {
      new(reinterpret_cast<T1*>(&mem.e1)) T1(e);
    }
    oneof(const T2 &e): valid(2) {
      new(reinterpret_cast<T2*>(&mem.e2)) T2(e);
    }
    oneof(const T3 &e): valid(3) {
      new(reinterpret_cast<T3*>(&mem.e3)) T3(e);
    }
    
    operator T1 &() { assert(valid == 1); return *reinterpret_cast<T1*>(&mem.e1); }
    operator const T1 &() const { assert(valid == 1); return *reinterpret_cast<const T1*>(&mem.e1); }
    operator T2 &() { assert(valid == 2); return *reinterpret_cast<T2*>(&mem.e2); }
    operator const T2 &() const { assert(valid == 2); return *reinterpret_cast<const T2*>(&mem.e2); }
    operator T3 &() { assert(valid == 3); return *reinterpret_cast<T3*>(&mem.e3); }
    operator const T3 &() const { assert(valid == 3); return *reinterpret_cast<const T3*>(&mem.e3); }
    
    void reset() {
      switch (valid) {
	case 1:
	  reinterpret_cast<T1*>(&mem.e1)->~T1();
	  break;
	case 2:
	  reinterpret_cast<T2*>(&mem.e2)->~T2();
	  break;
	case 3:
	  reinterpret_cast<T3*>(&mem.e3)->~T3();
	  break;
      }
      valid = 0;
    }
    
    template <typename T>
    T &operator *() {


    }

    template <typename T>
    T *getMemory() {
      reset();
      return reinterpret_cast<T *>(&mem);
    }
    
    ~oneof() { reset(); }
};

#endif // _INCLUDED_ONEOF_HPP
