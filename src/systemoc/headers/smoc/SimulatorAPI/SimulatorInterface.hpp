// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_SIMULATORAPI_SIMULATORINTERFACE_HPP
#define _INCLUDED_SMOC_SIMULATORAPI_SIMULATORINTERFACE_HPP

#include "TaskInterface.hpp"
#include "FiringRuleInterface.hpp"
#include "PortInterfaces.hpp"

#include <boost/program_options/options_description.hpp>
#include <boost/program_options/variables_map.hpp>

#include <list>

namespace smoc { namespace SimulatorAPI {

  class SimulatorInterface {
  public:
    typedef SimulatorAPI::TaskInterface         TaskInterface;
    typedef SimulatorAPI::FiringRuleInterface   FiringRuleInterface;
    typedef SimulatorAPI::PortInInterface       PortInInterface;
    typedef SimulatorAPI::PortOutInterface      PortOutInterface;

    enum EnablementStatus {
      IS_DISABLED,
      MAYBE_ACTIVE,
      MUSTBE_ACTIVE
    };

    virtual void populateOptionsDescription(
        int &argc, char ** &argv,
        boost::program_options::options_description &pub,
        boost::program_options::options_description &priv) = 0;

    virtual EnablementStatus evaluateOptionsMap(
        boost::program_options::variables_map &vm) = 0;

    virtual void registerTask(
        TaskInterface                          *task,
        std::list<FiringRuleInterface *> const &firingRules) = 0;

    /// Ports correspond to messages. Here, we tell the simulator
    /// which messages must be routed.
    virtual void registerPort(PortInInterface *port) = 0;
    virtual void registerPort(PortOutInterface *port) = 0;

    virtual ~SimulatorInterface() {}
  };

} } // namespace smoc::SimulatorAPI

#endif /* _INCLUDED_SMOC_SIMULATORAPI_SIMULATORINTERFACE_HPP */
