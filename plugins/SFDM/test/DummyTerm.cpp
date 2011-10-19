// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/Log.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CCells.hpp"
#include "Mesh/FieldGroup.hpp"

#include "DummyTerm.hpp"

using namespace cf3::common;
using namespace cf3::Mesh;

namespace cf3 {
namespace SFDM {

ComponentBuilder<DummyTerm,Term,LibSFDM> DummyTerm_builder;

/////////////////////////////////////////////////////////////////////////////////////

DummyTerm::DummyTerm ( const std::string& name ) :
  Term(name)
{
}

/////////////////////////////////////////////////////////////////////////////////////

DummyTerm::~DummyTerm() {}

/////////////////////////////////////////////////////////////////////////////////////

void DummyTerm::execute()
{
  boost_foreach(CRegion::Ptr region, m_loop_regions)
  {
    boost_foreach(CCells& cells, find_components_recursively<CCells>(*region))
    {
      CFinfo << "      " << name() << " for cells [" << cells.uri().path() << "]" << CFendl;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
