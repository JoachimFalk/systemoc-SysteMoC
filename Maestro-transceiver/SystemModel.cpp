//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2012 Hardware-Software-CoDesign, University of
 * Erlangen-Nuremberg. All rights reserved.
 * 
 *   This program is free software; you can redistribute it and/or modify it under
 *   the terms of the GNU General Public License as published by the Free Software
 *   Foundation; either version 2 of the License, or (at your option) any later
 *   version.
 * 
 *   This program is distributed in the hope that it will be useful, but WITHOUT
 *   ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 *   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
 *   details.
 * 
 *   You should have received a copy of the GNU General Public License along with
 *   this program; if not, write to the Free Software Foundation, Inc., 59 Temple
 *   Place, Suite 330, Boston, MA 02111-1307 USA.
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


/*
 * Transceiver Model
 * v0.1
 *
 * started: June-2012
 * Rafael Rosales
 * Hardware-Software-Co-Design
 * University of Erlangen-Nuremberg
 *
 * change: rosales      -       22-June-2012 -       Project Created
 * change: rosales      -       26-June-2012 -       First empty AMS model
 * change: rosales      -       27-June-2012 -       Analog Component v1.0 - Scripted Power and Timing models
 *
 *
 * */

//SysteMoC
#include <systemoc/smoc_moc.hpp>
#include <systemoc/smoc_port.hpp>
#include <systemoc/smoc_fifo.hpp>
#include <systemoc/smoc_node_types.hpp>
#include <systemoc/smoc_config.h>

//Helper
#include <iostream>
#include <list>
#include <time.h>

#include <AMS_Component.hpp>

using namespace std; 
using namespace MetaMap;

/**
 * System Model
 *
 * *Dummy* RF Transceiver consisting of two cascaded amplifiers
 *
 */
class RFTransceiver : public smoc_graph {

private:

        Amplifier amplifier1;
        Amplifier amplifier2;
        PowerMonitor topPowerMonitor;
        Merger2_1<double> powerModelsMerger; //merges the output of the power models for the top power monitor
        Src src;
        Snk snk;


public:
        /**
         * Constructor
         */
        RFTransceiver(sc_module_name name, int i=1)
                : smoc_graph(name),
                  amplifier1("Preamplifier"),
                  amplifier2("Amplifier"),
                  src("src"),
                  snk("snk"),
                  topPowerMonitor("TopPMonitor"),
                  powerModelsMerger("PowerModelsMerger")
        {
          //Connect transceiver components
          connectNodePorts(src.output,amplifier1.analogAmplifier.functionalModel.input);
          connectNodePorts(amplifier1.analogAmplifier.functionalModel.output,amplifier2.analogAmplifier.functionalModel.input);
          connectNodePorts(amplifier2.analogAmplifier.functionalModel.output,snk.input);

          //Connect power models to top power monitor
          connectNodePorts(amplifier1.analogAmplifier.powerModelDist.out2,powerModelsMerger.in1);
          connectNodePorts(amplifier2.analogAmplifier.powerModelDist.out2,powerModelsMerger.in2);
          connectNodePorts(powerModelsMerger.out,topPowerMonitor.power);
        }

};

/*
 * Main method
 *
 * Instantiates the system model and starts SystemC simulation
 *
 */
int sc_main (int argc, char **argv) {

  int times = 1;
  if (argc == 2) {
    const int iterations = atoi(argv[1]);
    //assert(iterations < NUM_MAX_ITERATIONS);
    //from = NUM_MAX_ITERATIONS - iterations;
      times = iterations;
  }

  smoc_top_moc<RFTransceiver> fgraph("Trans",times);

  sc_start();

  return 0;
}
