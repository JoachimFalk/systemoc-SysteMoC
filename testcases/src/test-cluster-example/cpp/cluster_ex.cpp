// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2019 FAU -- Joachim Falk <joachim.falk@fau.de>
 * 
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later
 * version.
 * 
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 * details.
 * 
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 * Place, Suite 330, Boston, MA 02111-1307 USA.
 */

#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_graph.hpp>

#include "DynSwitch.hpp"
#include "StaticCluster.hpp"

template<int upsample_factor>
class m_top: public smoc_graph {
private:
  StaticCluster<upsample_factor> static_cluster;
  DynSwitch dyn_switch;

public:
  m_top<upsample_factor>(sc_core::sc_module_name name)
    : smoc_graph(name)
    , static_cluster("static_cluster", 5, 10)
    , dyn_switch("dyn_switch", 1000)
  {
    connectNodePorts<2>(dyn_switch.out1,static_cluster.in1);
    connectNodePorts<1>(dyn_switch.out2,static_cluster.in2);

    connectNodePorts<2>(static_cluster.out1,dyn_switch.in1);
    connectNodePorts<1>(static_cluster.out2,dyn_switch.in2);
  }

};

int sc_main (int argc, char **argv) {
  smoc_top_moc<m_top<10> > top("top");
  sc_core::sc_start();
  return 0;
}
