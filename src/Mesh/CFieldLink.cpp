// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/BasicExceptions.hpp"
#include "Common/CBuilder.hpp"
#include "Common/LibCommon.hpp"
#include "Common/XmlHelpers.hpp"

#include "Mesh/CFieldLink.hpp"
#include "Mesh/CField2.hpp"

namespace CF {
namespace Mesh {

  using namespace Common;
////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CFieldLink, CLink, LibCommon > CFieldLink_Builder;

////////////////////////////////////////////////////////////////////////////////

CFieldLink::CFieldLink ( const std::string& name) : CLink ( name )
{
}

////////////////////////////////////////////////////////////////////////////////

CFieldLink::~CFieldLink()
{
}

////////////////////////////////////////////////////////////////////////////////

void CFieldLink::link_to ( Component::Ptr lnkto )
{
  if ( is_null(lnkto) )
    throw BadValue(FromHere(), "Cannot link to null component");

  if (lnkto->is_link())
    throw SetupError(FromHere(), "Cannot link a CFieldLink to another CFieldLink");

  m_link_component = boost::dynamic_pointer_cast<CField2>(lnkto);
}

////////////////////////////////////////////////////////////////////////////////

} // Common
} // CF
