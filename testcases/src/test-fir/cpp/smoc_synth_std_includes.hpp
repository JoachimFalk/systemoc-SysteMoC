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

#ifndef _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
#define _INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP

#include <cmath>
#include <cstdlib>

#include <iostream>
#include <limits>
#include <vector>
using namespace std;


namespace initializer{

  vector<double> get_fir_params() {
    vector<double> retval;
    // vector [0,0,1]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(1);
    return retval;
  }

  vector<double> get_fir_data() {
    vector<double> retval;
    // vector [0,0,0]
    retval.push_back(0);
    retval.push_back(0);
    retval.push_back(0);
    return retval;
  }
                

};

#endif //_INCLUDED_SMOC_SYNTH_STD_INCLUDES_HPP
