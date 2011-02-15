// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionURI.hpp"

#include "Mesh/CMesh.hpp"

#include "Solver/CDiscretization.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

CDiscretization::CDiscretization ( const std::string& name  ) :
  CMethod ( name )
{
  // properties

  properties()["brief"]=std::string("Discretization Method component");
  properties()["description"]=std::string("Handles the discretization of the PDE's");
  
  // options
  m_properties.add_option(OptionComponent<CMesh>::create("Mesh","Mesh the Discretization Method will be applied to",&m_mesh))
    ->mark_basic()
    ->add_tag("mesh");
  
  /// @todo not necessary, solution.topology() should provide this information.

  // for old compatibility
  std::vector< URI > dummy;
  m_properties.add_option< OptionArrayT < URI > > ("Regions", "Regions to loop over", dummy);

  URI empty;
  m_properties.add_option< OptionURI > ("Mesh", "Mesh to descretize", empty);
}

////////////////////////////////////////////////////////////////////////////////

CDiscretization::~CDiscretization()
{
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
