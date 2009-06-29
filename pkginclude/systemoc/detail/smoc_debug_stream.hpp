//  -*- tab-width:8; intent-tabs-mode:nil;  c-basic-offset:2; -*-
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

#ifndef _INCLUDED_SMOC_DETAIL_DEBUG_STREAM_HPP
#define _INCLUDED_SMOC_DETAIL_DEBUG_STREAM_HPP

#include <systemoc/smoc_config.h>

#ifdef SYSTEMOC_DEBUG

#include <CoSupport/Streams/FilterOStream.hpp>
#include <CoSupport/Streams/IndentStreambuf.hpp>
#include <CoSupport/Streams/DebugStreambuf.hpp>

using CoSupport::Streams::Indent;
using CoSupport::Streams::Debug;

extern Debug EXPR;  // info, FSM, event, expr
extern Debug EVENT; // info, FSM, event
extern Debug FSM;   // info, FSM
extern Debug INFO;  // default

struct SmocDebugOstream
: public CoSupport::Streams::FilterOStream
{
  CoSupport::Streams::DebugStreambuf bufDbg;
  CoSupport::Streams::IndentStreambuf bufIdt;
  SmocDebugOstream(const Debug& level = INFO);
};

extern SmocDebugOstream outDbg;

#endif // SYSTEMOC_DEBUG

#endif // _INCLUDED_SMOC_DETAIL_DEBUG_STREAM_HPP
