#ifndef _INCLUDED_SMOC_FIRING_STATE_HPP
#define _INCLUDED_SMOC_FIRING_STATE_HPP

#include <vector>
#include <iostream>
#include <list>
#include <set>

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>

#include <CoSupport/SmartPtr/intrusive_refcount_ptr.hpp>
#include <CoSupport/Streams/stl_output_for_list.hpp>
#include <CoSupport/DataTypes/Facade.hpp>
#include <CoSupport/commondefs.h>

#include <systemc.h>

#include <systemoc/smoc_config.h>

#include "smoc_guard.hpp"
#include "smoc_func_call.hpp"

#include <boost/static_assert.hpp>

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

class ConnectorStateImpl;
DECL_INTRUSIVE_REFCOUNT_PTR(ConnectorStateImpl, PConnectorStateImpl);

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

protected:
  smoc_firing_state_base(const SmartPtr &p);

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
  smoc_hierarchical_state(const SmartPtr &p);
  
  smoc_hierarchical_state(const this_type &);
  this_type& operator=(const this_type &);

public:
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
  smoc_firing_state(const SmartPtr &p);
  
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
  smoc_and_state(const SmartPtr &p);
  
  smoc_and_state(const this_type &);
  this_type& operator=(const this_type &);

public:
  smoc_and_state(size_t part, const std::string& name = "");

  smoc_xor_state::Ref operator[](size_t p);

  ImplType *getImpl() const;
  using smoc_firing_state_base::operator=;
};

class smoc_connector_state
: public CoSupport::DataTypes::FacadeFoundation<
    smoc_connector_state,
    ConnectorStateImpl,
    smoc_firing_state_base
  >
{
public:
  typedef smoc_connector_state this_type;

protected:
  smoc_connector_state(const SmartPtr &p);

  smoc_connector_state(const this_type &);
  this_type& operator=(const this_type &);
  
public:
  smoc_connector_state();

  ImplType *getImpl() const;
  using smoc_firing_state_base::operator=;
};

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
  smoc_multi_state(const SmartPtr &p);
  
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
  /// @brief IO req. and guards
  smoc_activation_pattern ap;

  /// @brief Action
  smoc_action f;

public:
  /// @brief Constructor
  explicit smoc_transition_part(
      const smoc_activation_pattern &ap)
    : ap(ap) {}

  /// @brief Constructor
  explicit smoc_transition_part(
      const smoc_activation_pattern &ap,
      const smoc_action& f)
    : ap(ap), f(f) {}

  const smoc_activation_pattern& getActivationPattern() const
    { return ap; }

  const smoc_action& getAction() const
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
public:
  typedef smoc_transition this_type;
  
private:
  smoc_activation_pattern ap;
  smoc_interface_action   ia;

public:
  /// @brief Constructor
  explicit smoc_transition(
      const smoc_action &f,
      smoc_firing_state_base::ConstRef &t)
    : ap(Expr::literal(true)), ia(t,f) {}
  
  /// @brief Constructor
  explicit smoc_transition(
      const smoc_activation_pattern &ap,
      smoc_firing_state_base::ConstRef &t)
    : ap(ap), ia(t) {}
  
  /// @brief Constructor
  explicit smoc_transition(
      const smoc_transition_part &tp,
      smoc_firing_state_base::ConstRef &t)
    : ap(tp.getActivationPattern()),
      ia(t, tp.getAction()) {}
  
  const smoc_activation_pattern &getActivationPattern() const
    { return ap; }
  
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

inline
smoc_transition_part operator >> (
    const smoc_activation_pattern &ap,
    const smoc_func_call &f)
  { return smoc_transition_part(ap, f); }

inline
smoc_transition_part operator >> (
    const smoc_activation_pattern &ap,
    const smoc_accum_action &b)
  { return smoc_transition_part(ap, b.getAction()); }

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
      tp.getActivationPattern(),
      merge(tp.getAction(), f));
}

inline
smoc_transition operator >> (
    const smoc_func_call &f,
    smoc_firing_state_base::ConstRef &s)
  { return smoc_transition(f, s); }

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

inline
smoc_transition operator >> (
    const smoc_activation_pattern &ap,
    smoc_firing_state_base::ConstRef &s)
  { return smoc_transition(ap,s); }

inline
smoc_transition_part operator >> (
    const smoc_activation_pattern &ap,
    const smoc_sr_func_pair &fp)
  { return smoc_transition_part(ap,fp); }

inline
smoc_transition operator >> (
    const smoc_sr_func_pair &fp,
    smoc_firing_state_base::ConstRef &s)
  { return smoc_transition(fp,s); }

inline
smoc_sr_func_pair operator && (
    const smoc_func_call &g,
    const smoc_func_call &t)
  { return smoc_sr_func_pair(g, t); }

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
