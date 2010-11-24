// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/MPI/PEObjectWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

/**
  @file PEObjectWrapper.cpp
  @author Tamas Banyai
**/

////////////////////////////////////////////////////////////////////////////////

namespace CF {
  namespace Common {

Common::ObjectProvider < PEObjectWrapper, Component, LibCommon, NB_ARGS_1 >
PEObjectWrapper_Provider ( PEObjectWrapper::type_name() );

  } // namespace Common
} // namespace CF

////////////////////////////////////////////////////////////////////////////////



