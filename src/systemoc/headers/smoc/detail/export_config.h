// -*- tab-width:8; indent-tabs-mode:nil; c-basic-offset:2; -*-
// vim: set sw=2 ts=8 et:
/*
 * Copyright (c)
 *   2010 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2012 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2016 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2017 FAU -- Joachim Falk <joachim.falk@fau.de>
 *   2018 FAU -- Joachim Falk <joachim.falk@fau.de>
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

#ifndef _INCLUDED_SMOC_DETAIL_EXPORT_CONFIG_H
#define _INCLUDED_SMOC_DETAIL_EXPORT_CONFIG_H

#include <CoSupport/commondefs.h>

#if defined(SYSTEMOC_DLL_EXPORT)
  //  defined(SYSTEMOC_DLL_EXPORT)
# define SYSTEMOC_API    COSUPPORT_ATTRIBUTE_DLL_EXPORT
# define SYSTEMOC_LOCAL  COSUPPORT_ATTRIBUTE_DLL_LOCAL
#elif defined(SYSTEMOC_DLL_IMPORT)
  // !defined(SYSTEMOC_DLL_EXPORT) &&
  //  defined(SYSTEMOC_DLL_IMPORT)
# define SYSTEMOC_API    COSUPPORT_ATTRIBUTE_DLL_IMPORT
# define SYSTEMOC_LOCAL
#else
  // !defined(SYSTEMOC_DLL_EXPORT) &&
  // !defined(SYSTEMOC_DLL_IMPORT)
# define SYSTEMOC_API
# define SYSTEMOC_LOCAL
#endif

#endif /* _INCLUDED_SMOC_DETAIL_EXPORT_CONFIG_H */
