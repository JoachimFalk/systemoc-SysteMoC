// vim: set sw=2 ts=8:
/*
 * Copyright (c) 2004-2009 Hardware-Software-CoDesign, University of Erlangen-Nuremberg.
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
#include <unistd.h>
#include <smoc/smoc_simulation_ctx.hpp>
#include <sysc/kernel/sc_cmnhdr.h>
#include <sysc/kernel/sc_externs.h>
#include <string.h>
#include <jni.h>
#include "systemoc_plugin_jni_SysteMoC.h"
#include <systemoc/smoc_moc.hpp>
#include <systemc.h>

int smoc_elab_and_sim(int _argc, char *_argv[]) {
  
  SysteMoC::smoc_simulation_ctx ctx(_argc, _argv);
  return sc_core::sc_elab_and_sim(ctx.getArgc(), ctx.getArgv());  
}

// run simulation from Java without args
JNIEXPORT void JNICALL 
Java_systemoc_plugin_jni_SysteMoC_runSimulation(JNIEnv *env, jobject obj) {
  
  smoc_elab_and_sim(0, NULL);
}

// run simulation from Java with string args (default)
JNIEXPORT void JNICALL 
Java_systemoc_plugin_jni_SysteMoC_runSimulationWithStringParams(JNIEnv *env, jobject obj, jobjectArray stringArray) {


  jsize len = env->GetArrayLength(stringArray);
  char *params[len];

  for(int i=0; i< len; i++){

    jstring tmp = (jstring) env->GetObjectArrayElement(stringArray, i);
    const char *string = env->GetStringUTFChars(tmp, 0);
    
    params[i] = new char[128];
    //std::cout << string << std::endl;
    strcpy(params[i], string);
  }
  
  //elab_and_sim ruft sc_main auf
  smoc_elab_and_sim(len, params);
}

//call sc_stop
JNIEXPORT void JNICALL 
Java_systemoc_plugin_jni_SysteMoC_stopSimulation(JNIEnv *env, jobject obj) {
  
  std::cout << "sc_stop" << std::endl;
  sc_stop();
}

//call sc_start
JNIEXPORT void JNICALL 
Java_systemoc_plugin_jni_SysteMoC_startSimulation(JNIEnv *env, jobject obj) {

  std::cerr << "sc_start()" << std::endl;
  SysteMoC::smoc_simulation_ctx::jniEvent.notify();
  sc_start();
}

sc_event SysteMoC::smoc_simulation_ctx::jniEvent;