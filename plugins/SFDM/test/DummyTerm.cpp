// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"
#include "mesh/SpaceFields.hpp"

#include "DummyTerm.hpp"

using namespace cf3::common;
using namespace cf3::mesh;

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
  boost_foreach(Handle< Region > region, m_loop_regions)
  {
    boost_foreach(Cells& cells, find_components_recursively<Cells>(*region))
    {
      CFinfo << "      " << name() << " for cells [" << cells.uri().path() << "]" << CFendl;
    }
  }
}

/////////////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3

/////////////////////////////////////////////////////////////////////////////////////
