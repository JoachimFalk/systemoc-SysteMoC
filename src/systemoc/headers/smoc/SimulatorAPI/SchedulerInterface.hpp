// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Liyuan Zhang <liyuan.zhang@cs.fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2020 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_SIMULATORAPI_SCHEDULERINTERFACE_HPP
#define _INCLUDED_SMOC_SIMULATORAPI_SCHEDULERINTERFACE_HPP

namespace smoc { namespace SimulatorAPI {

  class TaskInterface;
  class FiringRuleInterface;

  class SchedulerInterface {
  public:
    // This must be implemented by the scheduler and will be called by the
    // SysteMoC task if task->getUseActivationCallback() returns true.
    virtual void notifyActivation(TaskInterface *task, bool activation) = 0;

    // This must be implemented by the scheduler and will be called by the
    // SysteMoC task to denote that the guard of fr is checked. The scheduler
    // should account for the overhead of the guard.
    virtual void checkFiringRule(TaskInterface *task, FiringRuleInterface *fr) = 0;

    // This must be implemented by the scheduler and will be called by the
    // SysteMoC task to denote that the action of fr is executed. The scheduler
    // must account for the execution time of the action.
    virtual void executeFiringRule(TaskInterface *task, FiringRuleInterface *fr) = 0;

    virtual ~SchedulerInterface() {}
  };

} } // namespace smoc::SimulatorAPI

#endif /* _INCLUDED_SMOC_SIMULATORAPI_SCHEDULERINTERFACE_HPP */
