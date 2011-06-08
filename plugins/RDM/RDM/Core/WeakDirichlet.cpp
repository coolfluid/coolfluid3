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

#include "RDM/Core/WeakDirichlet.hpp"

#include "RDM/Core/LinearAdv2D.hpp"    // to remove
#include "RDM/Core/LinearAdvSys2D.hpp" // to remove
#include "RDM/Core/RotationAdv2D.hpp"  // to remove
#include "RDM/Core/Burgers2D.hpp"      // to remove
#include "RDM/Core/Euler2D.hpp"        // to remove

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < WeakDirichlet, RDM::BoundaryTerm, LibCore > WeakDirichlet_Builder;

Common::ComponentBuilder < FaceLoopT< WeakDirichlet, LinearAdv2D>    , RDM::FaceLoop, LibCore > WeakDirichlet_LinearAdv2D_Builder;
Common::ComponentBuilder < FaceLoopT< WeakDirichlet, LinearAdvSys2D> , RDM::FaceLoop, LibCore > WeakDirichlet_LinearAdvSys2D_Builder;
Common::ComponentBuilder < FaceLoopT< WeakDirichlet, RotationAdv2D>  , RDM::FaceLoop, LibCore > WeakDirichlet_RotationAdv2D_Builder;
Common::ComponentBuilder < FaceLoopT< WeakDirichlet, Burgers2D>      , RDM::FaceLoop, LibCore > WeakDirichlet_Burgers2D_Builder;
Common::ComponentBuilder < FaceLoopT< WeakDirichlet, Euler2D>        , RDM::FaceLoop, LibCore > WeakDirichlet_Euler2D_Builder;

///////////////////////////////////////////////////////////////////////////////////////

WeakDirichlet::WeakDirichlet ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  regist_typeinfo(this);

  // options

  m_properties.add_option< OptionURI > ("solution","Solution",
                                        "Solution field where to apply the boundary condition",
                                        URI("cpath:"))
       ->attach_trigger ( boost::bind ( &WeakDirichlet::config_mesh,   this ) )
       ->mark_basic();

  m_properties["mesh"].as_option().attach_trigger ( boost::bind ( &WeakDirichlet::config_mesh, this ) );

  m_properties.add_option<
      OptionArrayT<std::string> > ("Functions",
                                   "Math function applied as Dirichlet boundary condition (vars x,y)",
                                   std::vector<std::string>())
      ->attach_trigger ( boost::bind ( &WeakDirichlet::config_function, this ) )
      ->mark_basic();

  function.variables("x,y,z");
}

////////////////////////////////////////////////////////////////////////////////

void WeakDirichlet::config_function()
{
  function.functions( m_properties["Functions"].value<std::vector<std::string> >() );
  function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void WeakDirichlet::config_mesh()
{
  cf_assert( is_not_null( m_mesh.lock() ) );

  URI sol_uri  = property("solution").value<URI>();
  solution = access_component_ptr(sol_uri)->as_ptr<CField>();
  if( is_null(solution.lock()) )
    solution = find_component_ptr_with_tag<CField>( *(m_mesh.lock()) , "solution" );

  if( is_null(solution.lock()) )
    throw CastingFailed (FromHere(),
                         "Could not find a solution field on mesh "
                         + m_mesh.lock()->uri().string() );

}

/////////////////////////////////////////////////////////////////////////////////////

void WeakDirichlet::execute()
{
  const std::string physics = physical_model().type();

  // get the element loop or create it if does not exist
  ElementLoop::Ptr loop;
  Common::Component::Ptr cloop = get_child_ptr( "LOOP" );
  if( is_null( cloop ) )
  {
    loop = build_component_abstract_type_reduced< FaceLoop >( "FaceLoopT<" + type_name() + "," + physics + ">" , "LOOP");
    add_component(loop);
  }
  else
    loop = cloop->as_ptr_checked<ElementLoop>();

  // loop on all regions configured by the user

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {

//    std::cout << "REGION [" << region->uri().string() << "]" << std::endl;

    loop->select_region( region );

    // loop all elements of this region

    loop->execute();
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

