// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2019 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#include <CoSupport/String/Concat.hpp>

#include <systemoc/smoc_config.h>

#include <smoc/smoc_state.hpp>
#include <smoc/smoc_graph.hpp>
#include <smoc/smoc_actor.hpp>
#include <smoc/detail/DebugOStream.hpp>

namespace smoc {

using CoSupport::String::Concat;

//smoc_graph::smoc_graph()
//  : smoc::Detail::GraphBase(sc_core::sc_gen_unique_name("smoc_graph"), nullptr)
//  { constructor(); }

smoc_graph::smoc_graph(const sc_core::sc_module_name &name)
  : smoc::Detail::GraphBase(name, nullptr)
  { constructor(); }

smoc_graph::smoc_graph(const sc_core::sc_module_name &name, smoc_state &state)
  : smoc::Detail::GraphBase(name, &state)
  { constructor(); }

void smoc_graph::constructor() {
#ifdef SYSTEMOC_ENABLE_MAESTRO
  this->setName(this->name());
#endif //SYSTEMOC_ENABLE_MAESTRO
  // if there is at least one active transition: execute it
  //run = smoc::Expr::till(ol) >> SMOC_CALL(smoc_graph::scheduleDDF) >> run;
}

void smoc_graph::before_end_of_elaboration() {
#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << "<smoc_graph::before_end_of_elaboration name=\"" << name() << "\">"
         << std::endl << smoc::Detail::Indent::Up;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
  
  smoc::Detail::GraphBase::before_end_of_elaboration();
//initDDF();

#ifdef SYSTEMOC_ENABLE_DEBUG
  if (smoc::Detail::outDbg.isVisible(smoc::Detail::Debug::High)) {
    smoc::Detail::outDbg << smoc::Detail::Indent::Down << "</smoc_graph::before_end_of_elaboration>" << std::endl;
  }
#endif // SYSTEMOC_ENABLE_DEBUG
}

void smoc_graph::disableActor(std::string const &actorName) {
  smoc_actor *obj = dynamic_cast<smoc_actor *>(getChild(actorName));
  assert(obj && "Oops, disableActor called but no such actor is present!");
  obj->setActive(false);
}

void smoc_graph::reEnableActor(std::string const &actorName) {
  smoc_actor *obj = dynamic_cast<smoc_actor *>(getChild(actorName));
  assert(obj && "Oops, reEnableActor called but no such actor is present!");
  obj->setActive(true);
}

} // namespace smoc
