// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/FindComponents.hpp"
 

#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"

#include "Solver/CPhysicalModel.hpp"

#include "RDM/Core/ElementLoop.hpp"

#include "RDM/NavierStokes/WallWeakBc.hpp"

#include "RDM/Core/Euler2D.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < WallWeakBc, RDM::BoundaryTerm, LibCore > WallWeakBc_Builder;

Common::ComponentBuilder < FaceLoop< WallWeakBc, Euler2D>, RDM::ElementLoop, LibCore > WallWeakBc_Euler2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////

WallWeakBc::WallWeakBc ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  // options

  m_properties.add_option< OptionURI > ("solution","Solution",
                                        "Solution field where to apply the boundary condition",
                                        URI("cpath:"))
       ->attach_trigger ( boost::bind ( &WallWeakBc::config_mesh,   this ) )
       ->mark_basic()
       ->add_tag("solution");

  m_properties["mesh"].as_option().attach_trigger ( boost::bind ( &WallWeakBc::config_mesh, this ) );
}

////////////////////////////////////////////////////////////////////////////////

void WallWeakBc::config_mesh()
{
  cf_assert( is_not_null( m_mesh.lock() ) );

  URI sol_uri  = property("solution").value<URI>();
  solution = access_component_ptr(sol_uri)->as_ptr<CField>();
  if( is_null(solution.lock()) )
    solution = find_component_ptr_with_tag<CField>( *(m_mesh.lock()) , "solution" );

  if( is_null(solution.lock()) )
    throw CastingFailed (FromHere(),
                         "Could not find a solution field on mesh "
                         + m_mesh.lock()->full_path().string() );

}

/////////////////////////////////////////////////////////////////////////////////////

void WallWeakBc::execute()
{
  CPhysicalModel::Ptr pm = access_physical_model();

  const std::string physics = pm->type();

  // get the element loop or create it if does not exist
  ElementLoop::Ptr loop;
  Common::Component::Ptr cloop = get_child_ptr( "LOOP" );
  if( is_null( cloop ) )
  {
    loop = build_component_abstract_type< ElementLoop >( "CF.RDM.Core.FaceLoop<" + type_name() + "," + physics + ">" , "LOOP");
    add_component(loop);
  }
  else
    loop = cloop->as_ptr_checked<ElementLoop>();

  // loop on all regions configured by the user

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {

//    std::cout << "REGION [" << region->full_path().string() << "]" << std::endl;

    loop->select_region( region );

    // loop all elements of this region

    loop->execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

