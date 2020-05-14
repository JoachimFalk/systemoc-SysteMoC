// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2013 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2015 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Simone Müller <simone.mueller@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_CHANBASE_HPP
#define _INCLUDED_SMOC_DETAIL_CHANBASE_HPP

#include "../../systemoc/detail/smoc_port_registry.hpp"
//#include "../smoc_event.hpp"
#include "NamedIdedObj.hpp"

#include <systemoc/smoc_config.h>

#include <CoSupport/SystemC/ChannelModificationListener.hpp>

#include <systemc>

#include <list>

class smoc_reset_chan;

namespace smoc { namespace Detail {

class GraphBase;

class ChanBase
: public sc_core::sc_prim_channel,
  public smoc_port_registry,
#ifdef SYSTEMOC_NEED_IDS
  public smoc::Detail::NamedIdedObj,
#endif // SYSTEMOC_NEED_IDS
  public smoc::Detail::SimCTXBase
{
  typedef ChanBase this_type;
  friend class GraphBase; // reset
  friend class ::smoc_reset_chan; // reset
public:
#if defined(SYSTEMOC_NEED_IDS) || !defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
  // To reflect the generated name or SystemC name back to the NamedIdedObj base class
  // or to replace the SystemC name with out generated name.
  const char *name() const
# if !defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
    { assert(myName != ""); return myName.c_str(); }
# else //defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
    { return this->sc_core::sc_prim_channel::name(); }
# endif //defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
#endif // defined(SYSTEMOC_NEED_IDS) || !defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
protected:
  ChanBase(const std::string &name);

  virtual void setChannelID( std::string sourceActor,
                             CoSupport::SystemC::ChannelId id,
                             std::string name ) {};
 
  /// @brief generate channel names
  virtual void before_end_of_elaboration();

  /// @brief resets FIFOs
  virtual void start_of_simulation();

  virtual void doReset();

  virtual ~ChanBase();
private:
#ifndef SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP
  std::string myName; // patched in before_end_of_elaboration

  void generateName();
#endif //!defined(SYSTEMOC_ENABLE_MAESTROMM_SPEEDUP)
};

typedef std::list<ChanBase *> smoc_chan_list;

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_CHANBASE_HPP */
