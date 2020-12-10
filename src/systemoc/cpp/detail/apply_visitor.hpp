// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_APPLY_VISITOR_HPP
#define _INCLUDED_SMOC_DETAIL_APPLY_VISITOR_HPP

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/remove_const.hpp>
#include <smoc/detail/ChanBase.hpp>
#include <smoc/detail/FifoChanBase.hpp>
#include <smoc/detail/NodeBase.hpp>
#include <smoc/detail/PortBase.hpp>
#include <smoc/detail/RegisterChanBase.hpp>
#include <smoc/smoc_graph.hpp>
#include <smoc/smoc_actor.hpp>

#include <systemoc/smoc_multiplex_fifo.hpp>
#include <systemoc/smoc_multireader_fifo.hpp>
#include <systemoc/smoc_reset.hpp>

namespace smoc {

#define _SMOC_GENERATE_APPLY_VISITOR(TYPE)                                      \
template<typename Visitor>                                                      \
typename Visitor::result_type                                                   \
apply_visitor(Visitor &visitor, typename boost::remove_const<TYPE>::type &obj) {\
  return smoc::Detail::apply_visitor_helper                                     \
    <boost::remove_const>(visitor,&obj);                                        \
}                                                                               \
template <typename Visitor>                                                     \
typename Visitor::result_type                                                   \
apply_visitor(Visitor &visitor, typename boost::add_const<TYPE>::type &obj) {   \
  return smoc::Detail::apply_visitor_helper                                     \
    <boost::add_const>(visitor,&obj);                                           \
}
#define _SMOC_HANDLE_DERIVED_CLASS(TYPE)                                        \
  if (dynamic_cast<typename M<TYPE>::type *>(ptr))                              \
    return apply_visitor_helper<M>                                              \
      (visitor, static_cast<typename M<TYPE>::type *>(ptr))

/* smoc_graph */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_graph>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_graph)

/* GraphBase */

namespace Detail {

  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<GraphBase>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(smoc_graph);
    assert(!"WTF?! Unhandled derived class of smoc::Detail::GraphBase!");
  }

  _SMOC_GENERATE_APPLY_VISITOR(GraphBase)

} // namespace Detail

/* smoc_actor */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_actor>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_actor)

/* smoc_root_node */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc::Detail::NodeBase>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(smoc_actor);
    _SMOC_HANDLE_DERIVED_CLASS(GraphBase);
    assert(!"WTF?! Unhandled derived class of smoc_root_node!");
  }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc::Detail::NodeBase)

/* smoc_fifo */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<FifoChanBase>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(Detail::FifoChanBase)

/* smoc_multiplex_fifo_chan_base */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_multiplex_fifo_chan_base>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_multiplex_fifo_chan_base)

/* smoc_multireader_fifo_chan_base */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_multireader_fifo_chan_base>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_multireader_fifo_chan_base)

/* smoc_register */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<RegisterChanBase>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(Detail::RegisterChanBase)

/* smoc_reset_chan */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_reset_chan>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_reset_chan)

/* smoc_root_chan */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<ChanBase>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(FifoChanBase);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_multiplex_fifo_chan_base);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_multireader_fifo_chan_base);
    _SMOC_HANDLE_DERIVED_CLASS(RegisterChanBase);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_reset_chan);
    assert(!"Oops! Internal error: Unhandled derived class of smoc::Detail::ChanBase!");
  }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc::Detail::ChanBase)

/* smoc_sysc_port */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<PortBase>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(Detail::PortBase)

} // namespace smoc

namespace smoc { namespace Detail {

  /* sc_port_base */
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<sc_core::sc_port_base>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(PortBase);
    return visitor(*ptr); // fallback
  }

  /* sc_module */
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<sc_core::sc_module>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(NodeBase);
    return visitor(*ptr); // fallback
  }

  /* sc_object */
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<sc_core::sc_object>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(sc_core::sc_module);
    _SMOC_HANDLE_DERIVED_CLASS(sc_core::sc_port_base);
    _SMOC_HANDLE_DERIVED_CLASS(ChanBase);
    return visitor(*ptr); // fallback
  }

} } // namespace smoc::Detail

namespace sc_core { // We need this for correct Koenig lookup of apply_visitor

_SMOC_GENERATE_APPLY_VISITOR(sc_port_base)

_SMOC_GENERATE_APPLY_VISITOR(sc_module)

_SMOC_GENERATE_APPLY_VISITOR(sc_object)

} // namespace sc_core

#undef _SMOC_GENERATE_APPLY_VISITOR
#undef _SMOC_HANDLE_DERIVED_CLASS

//FIXME: This is a hack as some smoc_xxx classes are still in the global namespace!
using smoc::apply_visitor;

#endif /* _INCLUDED_SMOC_DETAIL_APPLY_VISITOR_HPP */
