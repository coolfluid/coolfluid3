// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/RegistLibrary.hpp"

#include "Mesh/VTKXML/LibVTKXML.hpp"

namespace cf3 {
namespace Mesh {
namespace VTKXML {

cf3::common::RegistLibrary<LibVTKXML> libVTKXML;

////////////////////////////////////////////////////////////////////////////////

void LibVTKXML::initiate_impl()
{
}

void LibVTKXML::terminate_impl()
{
}

////////////////////////////////////////////////////////////////////////////////

} // VTKXML
} // Mesh
} // cf3
