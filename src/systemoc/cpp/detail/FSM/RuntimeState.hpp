// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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

#ifndef _INCLUDED_SMOC_DETAIL_FSM_RUNTIMESTATE_HPP
#define _INCLUDED_SMOC_DETAIL_FSM_RUNTIMESTATE_HPP

#include <smoc/detail/NamedIdedObj.hpp>
#include <smoc/detail/SimCTXBase.hpp>
#include <smoc/detail/NodeBase.hpp>

#include "RuntimeTransition.hpp"

#include <systemoc/smoc_config.h>

#include <string>

namespace smoc { namespace Detail { namespace FSM {

  typedef std::set<smoc::smoc_event_waiter*> EventWaiterSet;

  class RuntimeState
    :
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
      public Bruckner::Model::State,
#endif //defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
#ifdef SYSTEMOC_NEED_IDS
      public NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
      public SimCTXBase
  {
      typedef RuntimeState this_type;

      friend class RuntimeTransition;
  private:
    std::string           stateName;
    RuntimeTransitionList tl;
  public:
    RuntimeState(std::string const &name
#if defined(SYSTEMOC_ENABLE_MAESTRO) && defined(MAESTRO_ENABLE_BRUCKNER)
        , Bruckner::Model::Hierarchical* sParent = nullptr
#endif
      );

    RuntimeTransitionList       &getTransitions();
    RuntimeTransitionList const &getTransitions() const;

    void addTransition(const RuntimeTransition& t,
                       NodeBase *node);

    void end_of_elaboration(NodeBase *node);

    EventWaiterSet am;

    const char *name()
      { return stateName.c_str(); }

    ~RuntimeState();
  private:
#ifdef SYSTEMOC_NEED_IDS
    // To reflect stateName back to NamedIdedObj base class.
    const char *name() const
      { return stateName.c_str(); }
#endif // SYSTEMOC_NEED_IDS
  };

} } } // namespace smoc::Detail::FSM

#endif /* _INCLUDED_SMOC_DETAIL_FSM_RUNTIMESTATE_HPP */
