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

//#include <boost/logic/tribool.hpp>
//#include <boost/intrusive_ptr.hpp>
//
//using boost::logic::tribool;
//using boost::logic::indeterminate;

class smoc_activation_pattern {
public:
  typedef smoc_activation_pattern this_type;
  
  friend class smoc_firing_state;
//protected:
  Expr::Ex<smoc_root_port_bool>::type guard;

  static
  void guardAssemble( smoc_modes::PGWriter &pgw, const Expr::PASTNode &n );
public:
  smoc_root_port_bool knownSatisfiable() const
    { return  Expr::evalTo<Expr::Value>(guard); }
//  smoc_root_port_bool knownUnsatisfiable() const
//    { return !Expr::evalTo<Expr::Value>(guard); }
  
  this_type onlyInputs()  const { return *this; }
  this_type onlyOutputs() const { return *this; }
  
  smoc_activation_pattern(bool v = false)
    : guard(Expr::literal(v)) {}
  
  template <class E>
  smoc_activation_pattern(const Expr::D<E> &guard)
    : guard(guard) {}
  
  template <class E>
  smoc_activation_pattern(const Expr::D<E> &guard, bool dummy)
    : guard(guard) {}

  void guardAssemble( smoc_modes::PGWriter &pgw ) const
    { guardAssemble(pgw, Expr::evalTo<Expr::AST>(guard) ); }
  
//  this_type concat( const smoc_activation_pattern &ap ) const
//    { return this_type(guard && ap.guard); }
  
  void dump(std::ostream &out) const;
};

static inline
std::ostream &operator <<( std::ostream &out, const smoc_activation_pattern &ap)
  { ap.dump(out); return out; }

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
