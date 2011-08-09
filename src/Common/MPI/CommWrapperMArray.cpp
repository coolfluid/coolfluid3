// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/CBuilder.hpp"
#include "Common/MPI/CommWrapperMultiArray.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

//////////////////////////////////////////////////////////////////////////////

ComponentBuilder < CommWrapperMultiArray<Uint,1>, CommWrapper, LibCommon > CommWrapperMultiArray_Uint_1_builder;
ComponentBuilder < CommWrapperMultiArray<int,1>,  CommWrapper, LibCommon > CommWrapperMultiArray_int_1_builder;
ComponentBuilder < CommWrapperMultiArray<Real,1>, CommWrapper, LibCommon > CommWrapperMultiArray_Real_1_builder;
//ComponentBuilder < CommWrapperMultiArray<bool,1>, CommWrapper, LibCommon > CommWrapperMultiArray_bool_1_builder;

ComponentBuilder < CommWrapperMultiArray<Uint,2>, CommWrapper, LibCommon > CommWrapperMultiArray_Uint_2_builder;
ComponentBuilder < CommWrapperMultiArray<int,2>,  CommWrapper, LibCommon > CommWrapperMultiArray_int_2_builder;
ComponentBuilder < CommWrapperMultiArray<Real,2>, CommWrapper, LibCommon > CommWrapperMultiArray_Real_2_builder;
//ComponentBuilder < CommWrapperMultiArray<bool,2>, CommWrapper, LibCommon > CommWrapperMultiArray_bool_2_builder;

////////////////////////////////////////////////////////////////////////////////

}
}

