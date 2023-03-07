// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2010 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2011 FAU -- Martin Streubuehr <martin.streubuehr@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Rafael Rosales <rafael.rosales@fau.de>
 *   2015 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Martin Letras <martin.letras@fau.de>
 *   2017 FAU -- Matthias Schid <matthias.schid@fau.de>
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

#ifndef _INCLUDED_SYSTEMOC_SMOC_MOC_HPP
#define _INCLUDED_SYSTEMOC_SMOC_MOC_HPP

#include <systemc>

#include <systemoc/smoc_config.h>

// This should be the main header for SysteMoC without smoc namespace.
// If there are additional headers required, please add them here.
#define _SYSTEMOC_NO_DEPRECATED_HEADER_WARNING

#include "smoc_event.hpp"
#include "smoc_port.hpp"
#include "smoc_fifo.hpp"
#include "smoc_multireader_fifo.hpp"
#include "smoc_multiplex_fifo.hpp"
#include "smoc_register.hpp"
#include "smoc_firing_rules.hpp"
#include "smoc_actor.hpp"
#include "smoc_periodic_actor.hpp"
#include "smoc_graph.hpp"
#include "smoc_graph_tt.hpp"
#include "smoc_scheduler_top.hpp"

#undef _SYSTEMOC_NO_DEPRECATED_HEADER_WARNING

//// for compatibility...
//typedef smoc_scheduler_top smoc_top;

template <typename Graph>
class smoc_top_moc : public Graph {
public:

  // -jens-
  // FIXME: this copies given parameter and makes it impossible to use
  //   references in constructor of GraphBase! Copying is evil anyway (much
  //   data, pointers, references, copy constructor has to exist, ...)

  smoc_top_moc()
    : Graph(), s(this) {}
  explicit smoc_top_moc(sc_core::sc_module_name name)
    : Graph(name), s(this) {}
  template <typename T1>
  explicit smoc_top_moc(sc_core::sc_module_name name, T1 p1)
    : Graph(name, p1), s(this) {}
  template <typename T1, typename T2>
  explicit smoc_top_moc(sc_core::sc_module_name name, T1 p1, T2 p2)
    : Graph(name, p1, p2), s(this) {}
  template <typename T1, typename T2, typename T3>
  explicit smoc_top_moc(sc_core::sc_module_name name, T1 p1, T2 p2, T3 p3)
    : Graph(name, p1, p2, p3), s(this) {}
  template <typename T1, typename T2, typename T3, typename T4>
  explicit smoc_top_moc(sc_core::sc_module_name name, T1 p1, T2 p2, T3 p3, T4 p4)
    : Graph(name, p1, p2, p3, p4), s(this) {}
  template <typename T1, typename T2, typename T3, typename T4, typename T5>
  explicit smoc_top_moc(sc_core::sc_module_name name, T1 p1, T2 p2, T3 p3, T4 p4, T5 p5)
    : Graph(name, p1, p2, p3, p4, p5), s(this) {}
private:
  smoc_scheduler_top s;
};

#endif /* _INCLUDED_SYSTEMOC_SMOC_MOC_HPP */
