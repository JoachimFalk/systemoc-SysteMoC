// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * SysteMoCScheduler.cpp
 *
 *  Created on: 09.05.2017
 *      Author: muellersi
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

  void registerTask(TaskInterface *task);

  void registerPort(TaskInterface *task, PortInInterface *port);
  void registerPort(TaskInterface *task, PortOutInterface *port);

  void simulationEnded();

  ~SysteMoCSimulator();
};

} } // namespace smoc::Detail
