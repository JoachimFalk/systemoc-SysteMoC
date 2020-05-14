// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
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

#include <smoc/SimulatorAPI/SimulatorInterface.hpp>

namespace smoc { namespace Detail {

class SysteMoCSimulator
  : public SimulatorAPI::SimulatorInterface
{
public:
  SysteMoCSimulator();

  void populateOptionsDescription(
      int &argc, char ** &argv,
      boost::program_options::options_description &pub,
      boost::program_options::options_description &priv);

  EnablementStatus evaluateOptionsMap(
      boost::program_options::variables_map &vm);

  void registerTask(
      TaskInterface                          *task,
      std::list<FiringRuleInterface *> const &firingRules);

  /// Ports correspond to messages. Here, we tell the simulator
  /// which messages must be routed.
  void registerPort(PortInInterface *port);
  void registerPort(PortOutInterface *port);

  ~SysteMoCSimulator();
};

} } // namespace smoc::Detail
