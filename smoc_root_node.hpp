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

#ifndef _INCLUDED_SMOC_ROOT_NODE_HPP
#define _INCLUDED_SMOC_ROOT_NODE_HPP

#include <smoc_firing_rules.hpp>
#include <smoc_port.hpp>
//#include <smoc_op.hpp>
#ifndef __SCFE__
# include <smoc_pggen.hpp>
#endif

#include <smoc_expr.hpp>

//#include <oneof.hpp>

#include <systemc.h>

#include <list>

class smoc_opbase_node {
public:
  typedef smoc_opbase_node this_type;
protected:
  template <typename T>
  smoc_func_call call(
      void (T::*f)(),
      const char *name = NULL ) {
    return smoc_func_call(this, f, name);
  }
  template <typename T, class X>
  typename Expr::MemGuard<T,X>::type guard(
      T (X::*m)() const,
      const char *name = NULL) const {
    return Expr::guard(dynamic_cast<const X *>(this), m, name);
  }
  template <typename T>
  static
  typename Expr::Var<T>::type var(T &x, const char *name = NULL)
    { return Expr::var(x,name); }
  
  virtual ~smoc_opbase_node() {}
};

// smoc_opbase_node must be the first class from which smoc_root_node
// is derived. This requirement comes from the reinterpret_cast in
// smoc_func_xxx classes in smoc_firing_rules.hpp
class smoc_root_node
#ifndef __SCFE__
: public smoc_opbase_node,
  public smoc_modes::smoc_modes_base_structure {
#else
: public smoc_opbase_node {
#endif
private:
#ifndef NDEBUG
  bool _finalizeCalled;
#endif
  smoc_firing_state        _currentState;
  const smoc_firing_state &_initialState;
  
  const smoc_firing_state &_communicate();
protected:
  smoc_root_node(const smoc_firing_state &s);
  smoc_root_node(smoc_firing_state &s);
public:
  // FIXME: protection
  bool               is_v1_actor;
  smoc_firing_state  commstate;
  smoc_firing_state  nextState;
  smoc_event         vpc_event;
  
  Expr::Ex<smoc_root_port_bool>::type *_guard;
  
  virtual void finalise();
#ifndef __SCFE__
  virtual sc_module *myModule() = 0;
  const sc_module *myModule() const {
    return const_cast<smoc_root_node *>(this)->myModule();
  }
  
  virtual void pgAssemble( smoc_modes::PGWriter &, const smoc_root_node * ) const;
  void assemble( smoc_modes::PGWriter &pgw ) const;
#endif
  
  const smoc_port_list getPorts() const;
  
  std::ostream &dumpActor( std::ostream &o );
  
  const smoc_firing_state &currentState() const { return _currentState; }
  smoc_firing_state       &currentState()       { return _currentState; }
};

typedef std::list<smoc_root_node *> smoc_node_list;

#endif // _INCLUDED_SMOC_ROOT_NODE_HPP
