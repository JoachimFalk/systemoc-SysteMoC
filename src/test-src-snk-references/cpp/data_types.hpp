/*
 * data_types.hpp
 *
 *  Created on: Nov 16, 2016
 *      Author: letras
 */


#ifndef DATA_TYPES_H
#define DATA_TYPES_H 1

template<typename T, int SIZE>
  class Token {
  public:
    T Data[SIZE];
    Token(); // default constructor is needed for software synthese
    T &operator[](int index);
  };
template<typename T, int SIZE>
  inline std::ostream &operator <<(std::ostream &out,
      const Token<T, SIZE> &token) {
    out << "...";
    return out;
  }
#endif

