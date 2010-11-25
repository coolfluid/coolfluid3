// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/LibCommon.hpp"
#include "Common/LibraryRegisterBase.hpp"
#include "Common/CBuilder.hpp"

#include "Common/CLibrary.hpp"

using namespace CF::Common;

Common::ComponentBuilder < CLibrary, Component, LibCommon >
CLibrary_Builder ( CLibrary::type_name() );

CLibrary::CLibrary(const std::string & lib_name):
    Component(lib_name)
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CLibrary::~CLibrary()
{

}

////////////////////////////////////////////////////////////////////////////////

void CLibrary::set_library(LibraryRegisterBase *lib)
{
  m_library = lib;
}

////////////////////////////////////////////////////////////////////////////////

void CLibrary::define_config_properties ( CF::Common::PropertyList& props )
{

}
