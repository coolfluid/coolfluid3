// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/CBuilder.hpp"
#include "Common/MPI/PEObjectWrapperMultiArray.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Common {

//////////////////////////////////////////////////////////////////////////////

ComponentBuilder < PEObjectWrapperMultiArray<Uint,1>, PEObjectWrapper, LibCommon > PEObjectWrapperMultiArray_Uint_1_builder;
ComponentBuilder < PEObjectWrapperMultiArray<int,1>,  PEObjectWrapper, LibCommon > PEObjectWrapperMultiArray_int_1_builder;
ComponentBuilder < PEObjectWrapperMultiArray<Real,1>, PEObjectWrapper, LibCommon > PEObjectWrapperMultiArray_Real_1_builder;
//ComponentBuilder < PEObjectWrapperMultiArray<bool,1>, PEObjectWrapper, LibCommon > PEObjectWrapperMultiArray_bool_1_builder;

ComponentBuilder < PEObjectWrapperMultiArray<Uint,2>, PEObjectWrapper, LibCommon > PEObjectWrapperMultiArray_Uint_2_builder;
ComponentBuilder < PEObjectWrapperMultiArray<int,2>,  PEObjectWrapper, LibCommon > PEObjectWrapperMultiArray_int_2_builder;
ComponentBuilder < PEObjectWrapperMultiArray<Real,2>, PEObjectWrapper, LibCommon > PEObjectWrapperMultiArray_Real_2_builder;
//ComponentBuilder < PEObjectWrapperMultiArray<bool,2>, PEObjectWrapper, LibCommon > PEObjectWrapperMultiArray_bool_2_builder;

////////////////////////////////////////////////////////////////////////////////

}
}

