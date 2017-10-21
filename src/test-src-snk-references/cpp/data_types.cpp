/*
 * data_types.cpp
 *
 *  Created on: Apr 6, 2017
 *      Author: streit
 */

#include "data_types.hpp"

template<typename T, int SIZE>
  Token<T, SIZE>::Token() {

  }

template<typename T, int SIZE>
  T &Token<T, SIZE>::operator[](int index) {
    assert(0 <= index && index < SIZE);
    return Token<T, SIZE>::Data[index];
  }
