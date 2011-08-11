// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

////////////////////////////////////////////////////////////////////////////////

#include "Common/CBuilder.hpp"
#include "Common/MPI/PEObjectWrapper.hpp"

////////////////////////////////////////////////////////////////////////////////

/**
  @file PEObjectWrapper.cpp
  @author Tamas Banyai
**/

////////////////////////////////////////////////////////////////////////////////


namespace CF {
namespace Common {

//////////////////////////////////////////////////////////////////////////////

//ComponentBuilder < PEObjectWrapperPtr<bool>,  PEObjectWrapper, LibCommon > PEObjectWrapperPtr_bool_builder;
ComponentBuilder < PEObjectWrapperPtr<int>,   PEObjectWrapper, LibCommon > PEObjectWrapperPtr_int_builder;
ComponentBuilder < PEObjectWrapperPtr<Uint>,  PEObjectWrapper, LibCommon > PEObjectWrapperPtr_Uint_builder;
ComponentBuilder < PEObjectWrapperPtr<Real>,  PEObjectWrapper, LibCommon > PEObjectWrapperPtr_Real_builder;

//ComponentBuilder < PEObjectWrapperVector<bool>, PEObjectWrapper, LibCommon > PEObjectWrapperVector_bool_builder;
ComponentBuilder < PEObjectWrapperVector<int>,  PEObjectWrapper, LibCommon > PEObjectWrapperVector_int_builder;
ComponentBuilder < PEObjectWrapperVector<Uint>, PEObjectWrapper, LibCommon > PEObjectWrapperVector_Uint_builder;
ComponentBuilder < PEObjectWrapperVector<Real>, PEObjectWrapper, LibCommon > PEObjectWrapperVector_Real_builder;

//ComponentBuilder < PEObjectWrapperVectorWeakPtr<bool>, PEObjectWrapper, LibCommon > PEObjectWrapperVectorWeakPtr_bool_builder;
ComponentBuilder < PEObjectWrapperVectorWeakPtr<int>,  PEObjectWrapper, LibCommon > PEObjectWrapperVectorWeakPtr_int_builder;
ComponentBuilder < PEObjectWrapperVectorWeakPtr<Uint>, PEObjectWrapper, LibCommon > PEObjectWrapperVectorWeakPtr_Uint_builder;
ComponentBuilder < PEObjectWrapperVectorWeakPtr<Real>, PEObjectWrapper, LibCommon > PEObjectWrapperVectorWeakPtr_Real_builder;

//////////////////////////////////////////////////////////////////////////////

} // Common
} // CF

////////////////////////////////////////////////////////////////////////////////



