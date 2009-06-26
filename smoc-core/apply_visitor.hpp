//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
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

#ifndef _INCLUDED_SMOC_APPLY_VISITOR_HPP
#define _INCLUDED_SMOC_APPLY_VISITOR_HPP

#include <boost/type_traits/add_const.hpp>
#include <boost/type_traits/remove_const.hpp>

#include <systemoc/detail/smoc_root_node.hpp>
#include <systemoc/detail/smoc_root_chan.hpp>
#include <systemoc/detail/smoc_sysc_port.hpp>
#include <systemoc/smoc_actor.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_multiplex_fifo.hpp>
#include <systemoc/smoc_multireader_fifo.hpp>
#include <systemoc/smoc_graph_type.hpp>

#include "smoc_graph_synth.hpp"

namespace SysteMoC {

#define _SMOC_GENERATE_APPLY_VISITOR(TYPE)                                      \
template<typename Visitor>                                                      \
typename Visitor::result_type                                                   \
apply_visitor(Visitor &visitor, typename boost::remove_const<TYPE>::type &obj) {\
  return SysteMoC::Detail::apply_visitor_helper                                 \
    <boost::remove_const>(visitor,&obj);                                        \
}                                                                               \
template <typename Visitor>                                                     \
typename Visitor::result_type                                                   \
apply_visitor(Visitor &visitor, typename boost::add_const<TYPE>::type &obj) {   \
  return SysteMoC::Detail::apply_visitor_helper                                 \
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

/* smoc_graph_sr */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_graph_sr>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_graph_sr)

/* smoc_graph_synth */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_graph_synth>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_graph_synth)

/* smoc_graph_base */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_graph_base>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(smoc_graph);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_graph_sr);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_graph_synth);
    assert(!"WTF?! Unhandled derived class of smoc_graph_base!");
  }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_graph_base)

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
  apply_visitor_helper(Visitor &visitor, typename M<smoc_root_node>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(smoc_actor);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_graph_base);
    assert(!"WTF?! Unhandled derived class of smoc_root_node!");
  }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_root_node)

/* smoc_fifo_chan_base */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_fifo_chan_base>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_fifo_chan_base)

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

/* smoc_root_chan */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_root_chan>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(smoc_fifo_chan_base);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_multiplex_fifo_chan_base);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_multireader_fifo_chan_base);
    assert(!"WTF?! Unhandled derived class of smoc_root_chan!");
  }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_root_chan)

/* smoc_sysc_port */

namespace Detail {
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<smoc_sysc_port>::type *ptr)
    { return visitor(*ptr); }
} // namespace Detail

_SMOC_GENERATE_APPLY_VISITOR(smoc_sysc_port)

} // namespace SysteMoC

namespace SysteMoC { namespace Detail {

  /* sc_port_base */
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<sc_core::sc_port_base>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(smoc_sysc_port);
    return visitor(*ptr); // fallback
  }

  /* sc_module */
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<sc_core::sc_module>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(smoc_root_node);
    return visitor(*ptr); // fallback
  }

  /* sc_object */
  template<template <class> class M, class Visitor>
  typename Visitor::result_type
  apply_visitor_helper(Visitor &visitor, typename M<sc_core::sc_object>::type *ptr) {
    _SMOC_HANDLE_DERIVED_CLASS(sc_core::sc_module);
    _SMOC_HANDLE_DERIVED_CLASS(sc_core::sc_port_base);
    _SMOC_HANDLE_DERIVED_CLASS(smoc_root_chan);
    return visitor(*ptr); // fallback
  }

} } // namespace SysteMoC::Detail

namespace sc_core { // We need this for correct Koenig lookup of apply_visitor

_SMOC_GENERATE_APPLY_VISITOR(sc_port_base)

_SMOC_GENERATE_APPLY_VISITOR(sc_module)

_SMOC_GENERATE_APPLY_VISITOR(sc_object)

} // namespace sc_core

#undef _SMOC_GENERATE_APPLY_VISITOR
#undef _SMOC_HANDLE_DERIVED_CLASS

//FIXME: This is a hack as some smoc_xxx classes are still in the global namespace!
using SysteMoC::apply_visitor;

#endif // _INCLUDED_SMOC_APPLY_VISITOR_HPP
