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

#ifndef _INCLUDED_SMOC_SIMULATORAPI_PORTINTERFACES_HPP
#define _INCLUDED_SMOC_SIMULATORAPI_PORTINTERFACES_HPP

#include "ChannelInterfaces.hpp"

#include <vector>

namespace smoc { namespace SimulatorAPI {

  class PortInInterface {
    typedef PortInInterface this_type;
  public:
    void        setSchedulerInfo(void *schedulerInfo)
      { this->schedulerInfo = schedulerInfo; }
    void       *getSchedulerInfo() const
      { return this->schedulerInfo; }

    virtual const char *name() const = 0;
    virtual void        commStart(size_t n) = 0;
    virtual void        commExec(size_t n) = 0;

    virtual
    ChannelSourceInterface       *getSource() = 0;
    ChannelSourceInterface const *getSource() const
      { return const_cast<this_type *>(this)->getSource(); }
  protected:
    PortInInterface()
      : schedulerInfo(nullptr) {}

    virtual ~PortInInterface() {}
  private:
    // Opaque data pointer for the scheduler.
    void *schedulerInfo;
  };

  class PortOutInterface {
    typedef PortOutInterface this_type;
  public:
    void        setSchedulerInfo(void *schedulerInfo)
      { this->schedulerInfo = schedulerInfo; }
    void       *getSchedulerInfo() const
      { return this->schedulerInfo; }

    virtual const char *name() const = 0;
    virtual void        commStart(size_t n) = 0;
    virtual void        commExec(size_t n) = 0;

    virtual
    std::vector<ChannelSinkInterface       *> const &getSinks() = 0;
    std::vector<ChannelSinkInterface const *> const &getSinks() const {
      return reinterpret_cast<std::vector<ChannelSinkInterface const *> const &>(
          const_cast<this_type *>(this)->getSinks());
    }
  protected:
    PortOutInterface()
      : schedulerInfo(nullptr) {}

    virtual ~PortOutInterface() {}
  private:
    // Opaque data pointer for the scheduler.
    void *schedulerInfo;
  };

} } // namespace smoc::SimulatorAPI

#endif /* _INCLUDED_SMOC_SIMULATORAPI_PORTINTERFACES_HPP */
