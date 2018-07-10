// -*- tab-width:8; intent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c) 2004-2018 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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
