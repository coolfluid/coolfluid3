// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/ObjectProvider.hpp"
#include "Common/LibCommon.hpp"

#include "Common/NoProfiling.hpp"

using namespace CF::Common;

ObjectProvider < NoProfiling, CodeProfiler, LibCommon, NB_ARGS_0 >
NoProfiling_Provider ( NoProfiling::type_name() );


NoProfiling::NoProfiling()
{

}

NoProfiling::~NoProfiling()
{

}
