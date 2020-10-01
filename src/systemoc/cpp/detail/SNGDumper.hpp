// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
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

#ifndef _INCLUDED_SMOC_DETAIL_SNGDUMPER_HPP
#define _INCLUDED_SMOC_DETAIL_SNGDUMPER_HPP

#include <systemoc/smoc_config.h>

#include <smoc/detail/GraphBase.hpp>

#include <iostream>

namespace smoc { namespace Detail {

class SimulationContextSNGDumping {
protected:
  SimulationContextSNGDumping();

  std::ostream   *dumpSNGFile;
public:
  bool isSNGDumpingEnabled() const
    { return dumpSNGFile != nullptr; }
  std::ostream &getSNGDumpFile() const
    { return *dumpSNGFile; }
};

void dumpSNG(std::ostream &file, SimulationContextSNGDumping *simCTX, GraphBase &g);

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_SNGDUMPER_HPP */
