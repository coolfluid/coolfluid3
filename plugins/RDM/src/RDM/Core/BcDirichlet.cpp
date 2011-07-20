// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Log.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CList.hpp"

#include "RDM/Core/BcDirichlet.hpp"

// #include "Common/MPI/debug.hpp" // temporary

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BcDirichlet, RDM::BoundaryTerm, Core::LibCore > BcDirichlet_Builder;

///////////////////////////////////////////////////////////////////////////////////////

BcDirichlet::BcDirichlet ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  // options

  m_options.add_option< OptionURI > ("solution", URI("cpath:"))
      ->set_description("Solution field where to apply the boundary condition")
      ->set_pretty_name("Solution")
      ->attach_trigger ( boost::bind ( &BcDirichlet::config_mesh,   this ) )
      ->mark_basic();

  m_options["mesh"].attach_trigger ( boost::bind ( &BcDirichlet::config_mesh, this ) );

  m_options.add_option< OptionArrayT<std::string> > ("functions", std::vector<std::string>())
      ->set_description("Math function applied as Dirichlet boundary condition (vars x,y)")
      ->attach_trigger ( boost::bind ( &BcDirichlet::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");
}

////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::config_function()
{
  m_function.functions( m_options["functions"].value<std::vector<std::string> >() );
  m_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::config_mesh()
{
  cf_assert( is_not_null( m_mesh.lock() ) );

  URI sol_uri  = option("solution").value<URI>();
  m_solution = access_component_ptr(sol_uri)->as_ptr<CField>();
  if( is_null(m_solution.lock()) )
    m_solution = find_component_ptr_with_tag<CField>( *(m_mesh.lock()) , "solution" );

  if( is_null(m_solution.lock()) )
    throw CastingFailed (FromHere(),
                         "Could not find a solution field on mesh "
                         + m_mesh.lock()->uri().string() );

}

/////////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::execute()
{
  if (m_solution.expired())
    throw SetupError(FromHere(), "Mesh option in ["+uri().path()+"] was not set");

  CField& field = *m_solution.lock();

//  std::cout << "   field.size() == " << field.size() << std::endl;
//  std::cout << "   coordinates.size() == " << mesh().nodes().coordinates().size() << std::endl;

  std::vector<Real> vars( DIM_3D, 0.);

  RealVector return_val( field.data().row_size() );

  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {

    /// @warning BcDirichlet assumes that solution maps one to one with mesh.nodes()

//    std::cout << PERank << "  region \'" << region->uri().string() << "\'" << std::endl;
    boost_foreach(const Uint node, CElements::used_nodes(*region).array())
    {
      cf_assert(node < field.size());

      CTable<Real>::ConstRow coords = mesh().nodes().coordinates()[node];

      for (Uint i=0; i<coords.size(); ++i)
        vars[i] = coords[i];

      m_function.evaluate(vars,return_val);

      CTable<Real>::Row data_row = field[node];
      for (Uint i=0; i<data_row.size(); ++i)
        data_row[i] = return_val[i];
    }

  }

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

