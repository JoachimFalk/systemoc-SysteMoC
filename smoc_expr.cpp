// vim: set sw=2 ts=8:
/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Library General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU Library General Public
 * License along with this program; if not, write to the
 * Free Software Foundation, Inc.,
 * 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
 */
#include <smoc_expr.hpp>

namespace Expr {

namespace Detail {

#ifdef SYSTEMOC_DEBUG
std::ostream &operator <<( std::ostream &out, Expr::Detail::ActivationStatus s) {
  static const char *display[3] = { "DISABLED", "BLOCKED", "ENABLED" };
  
  assert(static_cast<size_t>(s.value+1) < sizeof(display)/sizeof(display[0]));
  out << display[s.value+1];
  return out;
}
#endif

} // namespace Expr::Detail

} // namespace Expr
