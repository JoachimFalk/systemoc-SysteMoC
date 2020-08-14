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

#ifndef _INCLUDED_SMOC_SIMULATORAPI_FIRINGRULEINTERFACE_HPP
#define _INCLUDED_SMOC_SIMULATORAPI_FIRINGRULEINTERFACE_HPP

#include "PortInterfaces.hpp"

#include <systemc>

#include <vector>
#include <string>

namespace smoc { namespace SimulatorAPI {

  typedef std::vector<std::string> FunctionNames;

  class FiringRuleInterface {
  public:
    typedef SimulatorAPI::FunctionNames FunctionNames;

    struct PortInInfo {
      PortInInfo(PortInInterface &in, size_t c, size_t r)
        : port(in), consumed(c), required(r) {}

      PortInInterface  &port;
      size_t const      consumed;
      size_t const      required;
    };
    typedef std::vector<PortInInfo> PortInInfos;

    PortInInfos const  &getPortInInfos() const
      { return portInInfos; }

    struct PortOutInfo {
      PortOutInfo(PortOutInterface &out, size_t p, size_t r)
        : port(out), produced(p), required(r) {}

      PortOutInterface &port;
      size_t const      produced;
      size_t const      required;
    };
    typedef std::vector<PortOutInfo> PortOutInfos;

    PortOutInfos const &getPortOutInfos() const
      { return portOutInfos; }

    void                  setSchedulerInfo(void *schedulerInfo)
      { this->schedulerInfo = schedulerInfo; }
    void                 *getSchedulerInfo() const
      { return this->schedulerInfo; }

    virtual FunctionNames getGuardNames() const = 0;

    virtual size_t        getGuardComplexity() const = 0;

    virtual FunctionNames getActionNames() const = 0;

    virtual ~FiringRuleInterface() {}
  protected:
    FiringRuleInterface()
      : schedulerInfo(nullptr) {}

    PortInInfos  portInInfos;
    PortOutInfos portOutInfos;
  private:
    // Opaque data pointer for the scheduler.
    void *schedulerInfo;
  };

} } // namespace smoc::SimulatorAPI

#endif /* _INCLUDED_SMOC_SIMULATORAPI_FIRINGRULEINTERFACE_HPP */
