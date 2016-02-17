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

#ifndef _INCLUDED_SMOC_ACTOR_HPP
#define _INCLUDED_SMOC_ACTOR_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_VPC
# include <systemcvpc/ScheduledTask.hpp>
#endif //SYSTEMOC_ENABLE_VPC

#ifdef SYSTEMOC_ENABLE_MAESTRO
# include <Maestro/MetaMap/SMoCActor.hpp>
# include <Maestro/MetaMap/includes.hpp>
#endif //SYSTEMOC_ENABLE_MAESTRO

#include "detail/smoc_root_node.hpp"

class smoc_actor :
#ifdef SYSTEMOC_ENABLE_VPC
  public SystemC_VPC::ScheduledTask,
#endif //SYSTEMOC_ENABLE_VPC
#ifdef SYSTEMOC_ENABLE_MAESTRO
  public MetaMap::SMoCActor,
#endif //SYSTEMOC_ENABLE_MAESTRO
  public smoc_root_node {
protected:
#ifdef SYSTEMOC_ENABLE_MAESTRO
  smoc_actor(sc_module_name name, smoc_hierarchical_state &s, unsigned int thread_stack_size = 0x20000, bool useLogFile = false);
  smoc_actor(smoc_hierarchical_state &s, unsigned int thread_stack_size = 0x20000, bool useLogFile = false);
#else
  smoc_actor(sc_module_name name, smoc_hierarchical_state &s);
  smoc_actor(smoc_hierarchical_state &s);
#endif

#ifdef SYSTEMOC_ENABLE_VPC
  void finaliseVpcLink();
#endif //SYSTEMOC_ENABLE_VPC
  virtual void setActivation(bool activation);
#ifdef SYSTEMOC_ENABLE_MAESTRO
  void initMMactor();

  double getLocalTimeDiff();
#endif //SYSTEMOC_ENABLE_MAESTRO
public:

#ifdef SYSTEMOC_ENABLE_MAESTRO
  void wait();
  void wait(double v, sc_time_unit tu );
  void wait(sc_event& waitEvent);
  void wait(sc_time sct );

  void localClockWait(sc_time sct);
  void localClockWait(double v, sc_time_unit tu);

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
#endif // _INCLUDED_SMOC_ACTOR_HPP
