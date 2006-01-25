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

//#include <oneof.hpp>

#include <systemc.h>

#include <list>
#include <cosupport/functor.hpp>

class smoc_opbase_node {
public:
  typedef smoc_opbase_node this_type;
protected:
  
  template<typename F>
  typename CoSupport::ParamAccumulator<smoc_member_func, CoSupport::Functor<void, F> >::accumulated_type
  call(const F &f, const char *name = "") {
    return typename CoSupport::ParamAccumulator<smoc_member_func, CoSupport::Functor<void, F> >::accumulated_type
	    (CoSupport::Functor<void, F>(this, f, name));
  }
  
  template<typename F>
  typename Expr::MemGuard<F>::type guard(const F &f, const char *name = "") const {
    return Expr::guard(this, f, name);
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
  
  const smoc_firing_state &communicate();
protected:
  smoc_root_node(const smoc_firing_state &s);
  smoc_root_node(smoc_firing_state &s);
public:
  // FIXME: protection
  bool               is_v1_actor;
  smoc_firing_state  commstate;
  smoc_firing_state  nextState;
  smoc_event         vpc_event;
  smoc_port_list     ports_setup;
  
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
