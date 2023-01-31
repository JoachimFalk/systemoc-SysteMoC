// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2017 FAU -- Franz-Josef Streit <franz-josef.streit@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA.
 */

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
