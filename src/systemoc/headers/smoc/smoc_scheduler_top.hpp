// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_SMOC_SCHEDULER_TOP_HPP
#define _INCLUDED_SMOC_SMOC_SCHEDULER_TOP_HPP

#include <systemc>

#include "detail/SimCTXBase.hpp"

namespace smoc {

namespace Detail {
  class GraphBase;
} // namespace Detail

class smoc_scheduler_top
: public sc_core::sc_module,
  public Detail::SimCTXBase {

  friend class Detail::GraphBase;
public:
  smoc_scheduler_top(Detail::GraphBase *g);
  smoc_scheduler_top(Detail::GraphBase &g);
  ~smoc_scheduler_top();

protected:
  void start_of_simulation();
  void end_of_simulation();
  void _before_end_of_elaboration();
  void _end_of_elaboration();

private:
  Detail::GraphBase *g;
  bool               simulation_running;
};

} // namespace smoc

#endif /* _INCLUDED_SMOC_SMOC_SCHEDULER_TOP_HPP */
