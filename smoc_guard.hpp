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

#ifndef _INCLUDED_SMOC_GUARD_HPP
#define _INCLUDED_SMOC_GUARD_HPP

#include <iostream>

#include <cassert>
#include <climits>
#include <cmath>

#include <list>
#include <cosupport/stl_output_for_list.hpp>

#include <systemc.h>

#include <smoc_expr.hpp>
#include <smoc_port.hpp>
#include <smoc_pggen.hpp>

class smoc_activation_pattern
: public smoc_event_and_list {
public:
  typedef smoc_activation_pattern this_type;
  
  friend class smoc_firing_state;

  enum status_t {
    DISABLED = -1,
    BLOCKED  =  0,
    ENABLED  =  1
  };
//protected:
  Expr::Ex<bool>::type  guard;
protected:
  static
  void guardAssemble( smoc_modes::PGWriter &pgw, const Expr::PASTNode &n );
public:
  template <class E>
  smoc_activation_pattern(const Expr::D<E> &_guard)
    : guard(_guard) {}

  void finalise() {
    Expr::evalTo<Expr::Sensitivity>(guard, *this);
#ifdef SYSTEMOC_DEBUG
    std::cerr << "smoc_activation_pattern::finalise()"
                <<  " this == " << this
                << ", " << *this << std::endl;
#endif
  }
 
  inline
  status_t smoc_activation_pattern::getStatus() const;

  void guardAssemble( smoc_modes::PGWriter &pgw ) const
    { guardAssemble(pgw, Expr::evalTo<Expr::AST>(guard) ); }
};

#ifdef SYSTEMOC_DEBUG
static inline
std::ostream &operator <<( std::ostream &out, smoc_activation_pattern::status_t s) {
  static const char *display[3] = { "DISABLED", "BLOCKED", "ENABLED" };
  
  assert(static_cast<size_t>(s+1) < sizeof(display)/sizeof(display[0]));
  out << display[s+1];
  return out;
}
#endif

inline
smoc_activation_pattern::status_t smoc_activation_pattern::getStatus() const {
  status_t retval;
  
  if (*this) {
    retval = Expr::evalTo<Expr::Value>(guard)
      ? ENABLED
      : DISABLED;
    Expr::evalTo<Expr::CommReset>(guard);
  } else
    retval = BLOCKED;
  return retval;
//  return *this
//    ? ( Expr::evalTo<Expr::Value>(guard) ? ENABLED : DISABLED )
//    : BLOCKED;
}

namespace Expr {

template <class A, class B>
static inline
typename DOpBinConstruct<A,B,DOpBinLAnd>::result_type
operator >> (const D<A> &a, const D<B> &b) {
  return DOpBinConstruct<A,B,DOpBinLAnd>::
    apply(a.getExpr(),b.getExpr());
}

}

#endif // _INCLUDED_SMOC_GUARD_HPP
