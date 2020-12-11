// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
 *
 * This library is free software; you can redistribute it and/or modify it under
 * the terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation; either version 2 of the License, or (at your option) any
 * later version.
 *
 * This library is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 * details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with this library; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#ifndef _INCLUDED_SMOC_SMOC_TOKEN_TRAITS_HPP
#define _INCLUDED_SMOC_SMOC_TOKEN_TRAITS_HPP

#include <cstddef>

namespace smoc {

  template <typename T>
  class smoc_default_token_traits {
  public:
    typedef T token_type;

    static size_t tokenSize();
  };

  template <typename T>
  inline
  size_t smoc_default_token_traits<T>::tokenSize()
    { return sizeof(T); }

  template <>
  inline
  size_t smoc_default_token_traits<void>::tokenSize()
    { return 0; }

  template <typename T>
  class smoc_token_traits: public smoc_default_token_traits<T> {};

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_TOKEN_TRAITS_HPP */
