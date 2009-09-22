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

#ifndef _INCLUDED_SMOC_HOOKING_HPP
#define _INCLUDED_SMOC_HOOKING_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_ENABLE_HOOKING

//#include <systemc.h>

#include <string>

#include <boost/function.hpp>
#include <boost/regex.hpp>

class smoc_actor;

namespace SysteMoC { namespace Hook {

  typedef boost::function<bool (smoc_actor *, const std::string &, const std::string &, const std::string &)> PreCallback;
  typedef boost::function<void (smoc_actor *, const std::string &, const std::string &, const std::string &)> PostCallback;

  namespace Detail {

    /// Specify a transition hooking rule and its pre and post callbacks
    struct TransitionHook {
      boost::regex srcState;
      boost::regex action;
      boost::regex dstState;
      PreCallback  preCallback;
      PostCallback postCallback;

      TransitionHook(
        const std::string &srcState, const std::string &action, const std::string &dstState,
        const PreCallback &pre, const PostCallback &post);
    };

    void addTransitionHook(smoc_actor *, const TransitionHook &);

  } // namespace Detail

  void addTransitionHook(smoc_actor *,
    const std::string &srcState, const std::string &action, const std::string &dstState,
    const PreCallback &pre, const PostCallback &post);

} } // namespace SysteMoC::Hook

#endif // SYSTEMOC_ENABLE_HOOKING

#endif // _INCLUDED_SMOC_HOOKING_HPP
