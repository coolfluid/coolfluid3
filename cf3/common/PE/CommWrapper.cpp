// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "common/Builder.hpp"
#include "common/PE/CommWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

/**
  @file CommWrapper.cpp
  @author Tamas Banyai
**/

////////////////////////////////////////////////////////////////////////////////


namespace cf3 {
namespace common {
namespace PE {

//////////////////////////////////////////////////////////////////////////////

//ComponentBuilder < CommWrapperPtr<bool>,  CommWrapper, LibCommon > CommWrapperPtr_bool_builder;
ComponentBuilder < CommWrapperPtr<int>,   CommWrapper, LibCommon > CommWrapperPtr_int_builder;
ComponentBuilder < CommWrapperPtr<Uint>,  CommWrapper, LibCommon > CommWrapperPtr_Uint_builder;
ComponentBuilder < CommWrapperPtr<Real>,  CommWrapper, LibCommon > CommWrapperPtr_Real_builder;

//ComponentBuilder < CommWrapperVector<bool>, CommWrapper, LibCommon > CommWrapperVector_bool_builder;
ComponentBuilder < CommWrapperVector<int>,  CommWrapper, LibCommon > CommWrapperVector_int_builder;
ComponentBuilder < CommWrapperVector<Uint>, CommWrapper, LibCommon > CommWrapperVector_Uint_builder;
ComponentBuilder < CommWrapperVector<Real>, CommWrapper, LibCommon > CommWrapperVector_Real_builder;

//////////////////////////////////////////////////////////////////////////////

} // PE
} // common
} // cf3

////////////////////////////////////////////////////////////////////////////////



