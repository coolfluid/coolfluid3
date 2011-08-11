// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/CBuilder.hpp"
#include "Common/MPI/CommWrapperMArray.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

//////////////////////////////////////////////////////////////////////////////

ComponentBuilder < CommWrapperMArray<Uint,1>, CommWrapper, LibCommon > CommWrapperMArray_Uint_1_builder;
ComponentBuilder < CommWrapperMArray<int,1>,  CommWrapper, LibCommon > CommWrapperMArray_int_1_builder;
ComponentBuilder < CommWrapperMArray<Real,1>, CommWrapper, LibCommon > CommWrapperMArray_Real_1_builder;
//ComponentBuilder < CommWrapperMArray<bool,1>, CommWrapper, LibCommon > CommWrapperMArray_bool_1_builder;

ComponentBuilder < CommWrapperMArray<Uint,2>, CommWrapper, LibCommon > CommWrapperMArray_Uint_2_builder;
ComponentBuilder < CommWrapperMArray<int,2>,  CommWrapper, LibCommon > CommWrapperMArray_int_2_builder;
ComponentBuilder < CommWrapperMArray<Real,2>, CommWrapper, LibCommon > CommWrapperMArray_Real_2_builder;
//ComponentBuilder < CommWrapperMArray<bool,2>, CommWrapper, LibCommon > CommWrapperMArray_bool_2_builder;

////////////////////////////////////////////////////////////////////////////////

}
}

