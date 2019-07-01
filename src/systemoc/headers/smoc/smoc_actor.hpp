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

#ifndef _INCLUDED_SMOC_SMOC_ACTOR_HPP
#define _INCLUDED_SMOC_SMOC_ACTOR_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/SMoCActor.hpp>
# include <Maestro/MetaMap/includes.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

// Not needed for actor, but users will likely require this.
#include "smoc_firing_state.hpp"
// Not needed for actor, but users will likely require this.
#include "smoc_transition.hpp"

#include "detail/NodeBase.hpp"

namespace smoc {

class smoc_actor: public Detail::NodeBase {
protected:
  smoc_actor(sc_core::sc_module_name name, smoc_state &s, unsigned int thread_stack_size = 0
#ifdef SYSTEMOC_ENABLE_MAESTRO
      , bool useLogFile = false
#endif //defined(SYSTEMOC_ENABLE_MAESTRO)
    );
//smoc_actor(smoc_hierarchical_state &s, unsigned int thread_stack_size = 0
//#ifdef SYSTEMOC_ENABLE_MAESTRO
//    , bool useLogFile = false
//#endif //defined(SYSTEMOC_ENABLE_MAESTRO)
//  );

//  void before_end_of_elaboration();

#ifdef SYSTEMOC_ENABLE_MAESTRO
  void initMMactor();

  double getLocalTimeDiff();
#endif //SYSTEMOC_ENABLE_MAESTRO
public:

#ifdef SYSTEMOC_ENABLE_MAESTRO
  /**
   * Delta cycle wait
   */
  void wait();

  /**
   * Timespan wait
   */
  void wait(double v, sc_time_unit tu);

  /**
   * Wait for event
   */
  void wait(sc_event& waitEvent);

  /**
   * Timespan wait
   */
  void wait(sc_time sct);

  /**
   * Timespan wait according to actor-local time (as oppossed to simulation global time)
   */
  void localClockWait(sc_time sct);

  /**
   * Timespan wait according to actor-local time (as oppossed to simulation global time)
   */
  void localClockWait(double v, sc_time_unit tu);

  /**
   * Wait for timespan or event
   */
  void wait(sc_time sct, sc_event& waitEvent);

  virtual bool testCanExecute();

  virtual bool canExecute();
  virtual void getCurrentTransition(MetaMap::Transition*& activeTransition);
  virtual void registerTransitionReadyListener(MetaMap::TransitionReadyListener& trListener);
  
# ifdef MAESTRO_ENABLE_POLYPHONIC
  virtual void registerThreadDoneListener(MetaMap::ThreadDoneListener& tdListener);
# endif
  
  virtual void sleep(sc_event& wakeUpEvent);
  virtual void execute();

  virtual bool isScheduled();

  virtual void setScheduled(bool set);
#endif // SYSTEMOC_ENABLE_MAESTRO
};

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_ACTOR_HPP */
