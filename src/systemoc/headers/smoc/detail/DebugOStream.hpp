// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_DEBUGOSTREAM_HPP
#define _INCLUDED_SMOC_DETAIL_DEBUGOSTREAM_HPP

#include <systemoc/smoc_config.h>

#include <CoSupport/Streams/NullStreambuf.hpp>
#include <CoSupport/Streams/DebugStreambuf.hpp>
#include <CoSupport/Streams/IndentStreambuf.hpp>
//#include <CoSupport/Streams/FilterOStream.hpp>
//#include <CoSupport/Streams/HeaderFooterStreambuf.hpp>

namespace smoc { namespace Detail {

using CoSupport::Streams::Debug;
using CoSupport::Streams::ScopedDebug;
using CoSupport::Streams::Indent;
using CoSupport::Streams::ScopedIndent;

typedef
#ifndef SYSTEMOC_ENABLE_DEBUG
  CoSupport::Streams::NullStreambuf::Stream<
#endif
    CoSupport::Streams::DebugStreambuf::Stream<
      CoSupport::Streams::IndentStreambuf::Stream<
    > >
#ifndef SYSTEMOC_ENABLE_DEBUG
  >
#endif
  DebugOStream;

/// Debug output stream
extern DebugOStream outDbg;
extern DebugOStream outDbgFSM;

} } // namespace smoc::Detail

#endif /* _INCLUDED_SMOC_DETAIL_DEBUGOSTREAM_HPP */
