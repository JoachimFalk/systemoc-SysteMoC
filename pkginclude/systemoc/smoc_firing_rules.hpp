/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
 * 
 *   This library is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU Lesser General Public License as published by the Free
 *   Software Foundation; either version 2 of the License, or (at your option) any
 *   later version.
 * 
 *   This library is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU Lesser General Public License
 *   along with this library; if not, write to the Free Software Foundation, Inc.,
 *   59 Temple Place, Suite 330, Boston, MA 02111-1307 USA.
 * 
 * --- This software and any associated documentation is provided "as is" 
 * 
 * IN NO EVENT SHALL HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG
 * BE LIABLE TO ANY PARTY FOR DIRECT, INDIRECT, SPECIAL, INCIDENTAL, OR
 * CONSEQUENTIAL DAMAGES ARISING OUT OF THE USE OF THIS SOFTWARE AND ITS
 * DOCUMENTATION, EVEN IF HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN
 * NUREMBERG HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF ERLANGEN NUREMBERG, SPECIFICALLY
 * DISCLAIMS ANY WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. THE SOFTWARE PROVIDED
 * HEREUNDER IS ON AN "AS IS" BASIS, AND HARDWARE-SOFTWARE-CODESIGN, UNIVERSITY OF
 * ERLANGEN NUREMBERG HAS NO OBLIGATION TO PROVIDE MAINTENANCE, SUPPORT, UPDATES,
 * ENHANCEMENTS, OR MODIFICATIONS.
 */

#ifndef _INCLUDED_SMOC_FIRING_STATE_HPP
#define _INCLUDED_SMOC_FIRING_STATE_HPP

#include <vector>
#include <iostream>
#include <list>
#include <set>

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>

#include <smoc/smoc_expr.hpp>

#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>
#include <CoSupport/Streams/stl_output_for_list.hpp>
#include <CoSupport/DataTypes/Facade.hpp>
#include <CoSupport/commondefs.h>

#include <systemc>

#include <systemoc/smoc_config.h>

#include "detail/smoc_func_call.hpp"

#include <boost/static_assert.hpp>

#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/SMoCActor.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

typedef smoc::Expr::Ex<bool>::type Guard;

// FIXME: this should be in Detail but conflicts with other
// Detail namespaces...

template <class T>
struct FT: public CoSupport::DataTypes::FacadeTraits<T> {};

class PartialTransition;

class FiringStateBaseImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(FiringStateBaseImpl, PFiringStateBaseImpl);

class HierarchicalStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(HierarchicalStateImpl, PHierarchicalStateImpl);

class FiringStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(FiringStateImpl, PFiringStateImpl);

class XORStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(XORStateImpl, PXORStateImpl);

class ANDStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(ANDStateImpl, PANDStateImpl);

class JunctionStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(JunctionStateImpl, PJunctionStateImpl);

class MultiStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(MultiStateImpl, PMultiStateImpl);

class smoc_transition_list;

class smoc_firing_state_base
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_firing_state_base,
    FiringStateBaseImpl
  >
{
public:
  typedef smoc_firing_state_base this_type;

  friend class FiringStateBaseImpl;
protected:
  explicit smoc_firing_state_base(_StorageType const &x): FFType(x) {}
  smoc_firing_state_base(SmartPtr const &p);
public:
  /// @brief Add transitions to state
  void addTransition(const smoc_transition_list &tl);

  /// @brief Clear all transitions
  void clearTransition();
  
  this_type& operator=(const smoc_transition_list &tl);
  this_type& operator|=(const smoc_transition_list &tl);
};

class smoc_hierarchical_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_hierarchical_state,
    HierarchicalStateImpl,
    smoc_firing_state_base
  >
{
  typedef smoc_hierarchical_state this_type;

protected:
  explicit smoc_hierarchical_state(_StorageType const &x): FFType(x) {}
  smoc_hierarchical_state(SmartPtr const &p);
  
  smoc_hierarchical_state(const this_type &);

  this_type& operator=(const this_type &);

public:

  this_type& clone(const this_type &);

  ImplType *getImpl() const;
  using smoc_firing_state_base::operator=;

  smoc_hierarchical_state::Ref select(
      const std::string& name);
  smoc_hierarchical_state::ConstRef select(
      const std::string& name) const;

  const std::string& getName() const;
  std::string getHierarchicalName() const;
};

class smoc_firing_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_firing_state,
    FiringStateImpl,
    smoc_hierarchical_state
  >
{
  typedef smoc_firing_state this_type;

protected:
  explicit smoc_firing_state(_StorageType const &x): FFType(x) {}
  smoc_firing_state(SmartPtr const &p);
  
  smoc_firing_state(const this_type &);
  this_type& operator=(const this_type &);

public:
  smoc_firing_state(const std::string& name = "");

  ImplType *getImpl() const;
  using smoc_firing_state_base::operator=;
};

class smoc_xor_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_xor_state,
    XORStateImpl,
    smoc_hierarchical_state
  >
{
public:
  typedef smoc_xor_state this_type;

protected:
  friend class smoc_and_state;
  
  explicit smoc_xor_state(_StorageType const &x): FFType(x) {}
  smoc_xor_state(const SmartPtr &p);

  smoc_xor_state(const this_type &);
  this_type& operator=(const this_type &);

public:
  smoc_xor_state(const std::string& name = "");
  smoc_xor_state(const smoc_hierarchical_state &init);

  this_type& add(const smoc_hierarchical_state &state);
  this_type& init(const smoc_hierarchical_state &state);

  ImplType *getImpl() const;
  using smoc_firing_state_base::operator=;
};

class smoc_and_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_and_state,
    ANDStateImpl,
    smoc_hierarchical_state
  >
{
public:
  typedef smoc_and_state this_type;

protected:
  explicit smoc_and_state(_StorageType const &x): FFType(x) {}
  smoc_and_state(const SmartPtr &p);
  
  smoc_and_state(const this_type &);
  this_type& operator=(const this_type &);

public:
  smoc_and_state(const std::string& name = "");

  this_type& add(const smoc_hierarchical_state &state);

  ImplType *getImpl() const;
  using smoc_firing_state_base::operator=;
};

class smoc_junction_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_junction_state,
    JunctionStateImpl,
    smoc_firing_state_base
  >
{
public:
  typedef smoc_junction_state this_type;

protected:
  explicit smoc_junction_state(_StorageType const &x): FFType(x) {}
  smoc_junction_state(SmartPtr const &p);

  smoc_junction_state(const this_type &);
  this_type& operator=(const this_type &);
  
public:
  smoc_junction_state();

  ImplType *getImpl() const;
  using smoc_firing_state_base::operator=;
};
#ifdef _MSC_VER
// Visual Studio defines "IN" 
#undef IN
#endif // _MSC_VER
struct IN {
  smoc_hierarchical_state::ConstRef s;
  bool neg;
  IN(const smoc_hierarchical_state& s)
    : s(s), neg(false) {}
  IN& operator!()
    { neg = !neg; return *this; }
};

class smoc_multi_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_multi_state,
    MultiStateImpl,
    smoc_firing_state_base
  >
{
public:
  typedef smoc_multi_state this_type;

protected:
  explicit smoc_multi_state(_StorageType const &x): FFType(x) {}
  smoc_multi_state(SmartPtr const &p);
  
  smoc_multi_state(const this_type &);
  this_type& operator=(const this_type &);

public:
  smoc_multi_state(const smoc_hierarchical_state& s);
  smoc_multi_state(const IN& s);

  this_type& operator,(const smoc_hierarchical_state& s);
  this_type& operator,(const IN& s);

  ImplType *getImpl() const;
  using smoc_firing_state_base::operator=;
};


class smoc_interface_action {
public:
  typedef smoc_interface_action this_type;

private:
  /// @brief Target state
  smoc_firing_state_base::ConstPtr dest;

  /// @brief Action
  smoc_action f;

public:
  /// @brief Constructor
  explicit smoc_interface_action(
      smoc_firing_state_base::ConstRef &t)
    : dest(t.toPtr()) {}

  /// @brief Constructor
  explicit smoc_interface_action(
      smoc_firing_state_base::ConstRef &t,
      const smoc_action &f)
    : dest(t.toPtr()), f(f) {}

  const smoc_firing_state_base::ConstPtr& getDestState() const
    { return dest; }

  const smoc_action& getAction() const
    { return f; }
};

class smoc_transition_part {
public:
  typedef smoc_transition_part this_type;

private:
  /// @brief guard (AST assembled from smoc_expr.hpp nodes)
  Guard guard;

  /// @brief Action
  smoc_action f;
public:
  /// @brief Constructor
  explicit smoc_transition_part(Guard const &g)
    : guard(g) {}

  /// @brief Constructor
  explicit smoc_transition_part(Guard const &g, const smoc_action &f)
    : guard(g), f(f) {}

  /// @brief Returns the guard
  Guard const &getExpr() const
    { return guard; }

  /// @brief Returns the action
  const smoc_action &getAction() const
    { return f; }
};

class smoc_accum_action {
private:
  /// @brief Action
  smoc_action f;

public:
  /// @brief Constructor
  explicit smoc_accum_action(
      const smoc_action& f)
    : f(f) {}

  const smoc_action& getAction() const
    { return f; }
};

class smoc_transition {
  typedef smoc_transition this_type;
private:
  Guard                   guard;
  smoc_interface_action   ia;
public:
  /// @brief Constructor
  explicit smoc_transition(
      const smoc_action &f,
      smoc_firing_state_base::ConstRef &t)
    : guard(smoc::Expr::literal(true)), ia(t,f) {}
  
  /// @brief Constructor
  explicit smoc_transition(
      Guard const &g,
      smoc_firing_state_base::ConstRef &t)
    : guard(g), ia(t) {}
  
  /// @brief Constructor
  explicit smoc_transition(
      const smoc_transition_part &tp,
      smoc_firing_state_base::ConstRef &t)
    : guard(tp.getExpr()),
      ia(t, tp.getAction()) {}
  
  /// @brief Returns the guard
  Guard const &getExpr() const
    { return guard; }

  /// @brief Returns the interface action
  const smoc_interface_action &getInterfaceAction() const
    { return ia; }
};



class smoc_transition_list
: public std::vector<smoc_transition> {
public:
  typedef smoc_transition_list this_type;
public:
  smoc_transition_list() {}

  smoc_transition_list(const smoc_transition &t)
    { push_back(t); }
  
  this_type &operator |= (const smoc_transition &t)
    { push_back(t); return *this; }
};



inline
smoc_transition_list operator | (
    const smoc_transition_list &tl,
    const smoc_transition &t )
  { return smoc_transition_list(tl) |= t; }

inline
smoc_transition_list operator | (
    const smoc_transition &tx,
    const smoc_transition &t )
  { return smoc_transition_list(tx) |= t; }

template <class E>
smoc_transition_part operator >> (
    const smoc::Expr::D<E> &g,
    const smoc_func_call &f)
  { return smoc_transition_part(Guard(g), f); }

template <class E>
smoc_transition_part operator >> (
    const smoc::Expr::D<E> &g,
    const smoc_accum_action &b)
  { return smoc_transition_part(Guard(g), b.getAction()); }

inline
smoc_accum_action operator >> (
    const smoc_func_call &a,
    const smoc_func_call &b)
  { return smoc_accum_action(merge(a, b)); }

inline
smoc_accum_action operator >> (
    const smoc_accum_action &a,
    const smoc_func_call &b)
  { return smoc_accum_action(merge(a.getAction(), b)); }

inline
smoc_transition_part operator >> (
    const smoc_transition_part &tp,
    const smoc_func_call &f) {
  return smoc_transition_part(
      tp.getExpr(),
      merge(tp.getAction(), f));
}

inline
smoc_transition operator >> (
    const smoc_func_call &f,
    smoc_firing_state_base::ConstRef &s)
  { return smoc_transition(smoc_action(f), s); }

inline
smoc_transition operator >> (
    const smoc_accum_action &f,
    smoc_firing_state_base::ConstRef &s)
  { return smoc_transition(f.getAction(), s); }

inline
smoc_transition operator >> (
    const smoc_transition_part &tp,
    smoc_firing_state_base::ConstRef &s)
  { return smoc_transition(tp,s); }

template <class E>
smoc_transition operator >> (
    const smoc::Expr::D<E> &g,
    smoc_firing_state_base::ConstRef &s)
  { return smoc_transition(Guard(g),s); }

//template <class E>
//smoc_transition_part operator >> (
//    const smoc::Expr::D<E> &g,
//    const smoc_sr_func_pair &fp)
//  { return smoc_transition_part(Guard(g),fp); }
//
//inline
//smoc_transition operator >> (
//    const smoc_sr_func_pair &fp,
//    smoc_firing_state_base::ConstRef &s)
//  { return smoc_transition(smoc_action(fp),s); }
//
//inline
//smoc_sr_func_pair operator && (
//    const smoc_func_call &f1,
//    const smoc_func_call &f2)
//  { return smoc_sr_func_pair(f1, f2); }

inline
smoc_multi_state::Ref operator,(
    const smoc_hierarchical_state& a,
    const smoc_hierarchical_state& b)
  { return smoc_multi_state(a),b; }

inline
smoc_multi_state::Ref operator,(
    const IN& a,
    const smoc_hierarchical_state& b)
  { return smoc_multi_state(a),b; }

inline
smoc_multi_state::Ref operator,(
    const smoc_hierarchical_state& a,
    const IN& b)
  { return smoc_multi_state(a),b; }

inline
smoc_multi_state::Ref operator,(
    const IN& a,
    const IN& b)
  { return smoc_multi_state(a),b; }

#endif // _INCLUDED_SMOC_FIRING_STATE_HPP
