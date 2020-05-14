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

#ifndef _INCLUDED_SMOC_SIMULATORAPI_TRANSITIONINTERFACE_HPP
#define _INCLUDED_SMOC_SIMULATORAPI_TRANSITIONINTERFACE_HPP

#include "FiringRuleInterface.hpp"

#include <list>

namespace smoc { namespace SimulatorAPI {

  // This is a placeholder for FSM::RuntimeTransition. No other class
  // must be derived from TransitionInfo.
  class TransitionInterface {
  public:
    FiringRuleInterface *getFiringRule() const
      { return firingRule; }
  protected:
    FiringRuleInterface *firingRule;

    TransitionInterface(FiringRuleInterface *firingRule)
      : firingRule(firingRule) {}
    TransitionInterface(TransitionInterface const &ti)
      : firingRule(ti.firingRule) {}
    TransitionInterface &operator=(TransitionInterface const &ti)
      { firingRule = ti.firingRule; return *this; }
  };

  typedef std::list<TransitionInterface const> TransitionInterfaceList;

} } // namespace smoc::SimulatorAPI

#endif /* _INCLUDED_SMOC_SIMULATORAPI_TRANSITIONINTERFACE_HPP */
