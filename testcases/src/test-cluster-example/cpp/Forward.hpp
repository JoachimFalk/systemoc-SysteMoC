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

#ifndef _INCLUDED_FORWARD
#define _INCLUDED_FORWARD

#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_actor.hpp>

class Forward
  : public smoc_actor {

public:

  smoc_port_in<int> in1;

  smoc_port_out<int> out1;

private:

  const unsigned long heat_time;

  //waste some CPU memory
  void heat(){
    usleep(heat_time);

    out1[0] = in1[0];
  }

  smoc_firing_state fsm_main;

public:
  Forward(sc_core::sc_module_name name,
          unsigned long heat_time)
    : smoc_actor(name, fsm_main),
      heat_time(heat_time)
  {
    
    SMOC_REGISTER_CPARAM(heat_time);    

    fsm_main = 
      (in1(1) && out1(1)) >>
      SMOC_CALL(Forward::heat) >>
      fsm_main;

  }


};

#endif
