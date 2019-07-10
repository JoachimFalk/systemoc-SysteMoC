#ifndef _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
#define _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP


#include <assert.h>
#include <cstdlib>
#include <iostream>

template<typename T, int SIZE>
  class Token {
  public:
    T Data[SIZE];
    Token(){}; // default constructor is needed for software synthese
    T &operator[](int index)
    {
      assert(0 <= index && index < SIZE);
      return Token<T, SIZE>::Data[index];
    }
  };

template<typename T, int SIZE>
  inline std::ostream &operator <<(std::ostream &out,
      const Token<T, SIZE> &token) {
    out << "...";
    return out;
  }

#endif // _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
