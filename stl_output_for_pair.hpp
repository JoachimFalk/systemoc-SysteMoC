/* vim: set sw=2 ts=8: */
/*
 * Copyright (C) Joachim Falk <joachim.falk@gmx.de> $Date: 2003/01/24 13:49:25 $
 *
 * stl_output_for_pair.hpp is part of the jf_libs distribution of Joachim Falk;
 * you can redistribute it and/or modify it under the terms of the
 * GNU General Public License as published by the Free Software Foundation;
 * either version 2 of the License, or (at your option) any later version.
 *
 * The jf_libs distributio is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public
 * License along with this program; If not, write to
 * the Free Software Foundation, Inc., 59 Temple Place - Suite 330,
 * Boston, MA 02111-1307, USA.
 */

#ifndef _INCLUDED_STL_OUTPUT_FOR_PAIR_HPP
#define _INCLUDED_STL_OUTPUT_FOR_PAIR_HPP

#include <iostream>
#include <utility>
                                                                                
template <typename TA, typename TB>
std::ostream &operator <<(std::ostream &out, const std::pair<TA,TB> &p)
  { out << "[pair:" << p.first << "," << p.second << "]"; return out; }

#endif /* _INCLUDED_STL_OUTPUT_FOR_PAIR_HPP */
