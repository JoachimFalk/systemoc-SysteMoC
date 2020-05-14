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

#ifndef _INCLUDED_SMOC_DETAIL_SMXIMPORTER_HPP
#define _INCLUDED_SMOC_DETAIL_SMXIMPORTER_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_SGX

#include <istream>

namespace smoc { namespace Detail {

class SimulationContext;

class SimulationContextSMXImporting {
  friend void importSMX(SimulationContext *simCTX);
protected:
  SimulationContextSMXImporting();

  std::istream   *importSMXFile;
public:
  bool isSMXImportingEnabled() const
    { return importSMXFile; }
};

void importSMX(SimulationContext *simCTX);

} } // namespace smoc::Detail

#endif // SYSTEMOC_ENABLE_SGX

#endif /* _INCLUDED_SMOC_DETAIL_SMXIMPORTER_HPP */
