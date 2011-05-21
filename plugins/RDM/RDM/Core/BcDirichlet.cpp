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

#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CList.hpp"

#include "RDM/Core/BcDirichlet.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BcDirichlet, RDM::BoundaryTerm, LibCore > BcDirichlet_Builder;

///////////////////////////////////////////////////////////////////////////////////////

BcDirichlet::BcDirichlet ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  // options

  m_properties.add_option< OptionURI > ("solution","Solution",
                                        "Solution field where to apply the boundary condition",
                                        URI("cpath:"))
       ->attach_trigger ( boost::bind ( &BcDirichlet::config_mesh,   this ) )
       ->mark_basic();

  m_properties["mesh"].as_option().attach_trigger ( boost::bind ( &BcDirichlet::config_mesh, this ) );

  m_properties.add_option<
      OptionArrayT<std::string> > ("Functions",
                                   "Math function applied as Dirichlet boundary condition (vars x,y)",
                                   std::vector<std::string>())
      ->attach_trigger ( boost::bind ( &BcDirichlet::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");
}

////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::config_function()
{
  m_function.functions( m_properties["Functions"].value<std::vector<std::string> >() );
  m_function.parse();
}

////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::config_mesh()
{
  cf_assert( is_not_null( m_mesh.lock() ) );

  URI sol_uri  = property("solution").value<URI>();
  m_solution = access_component_ptr(sol_uri)->as_ptr<CField>();
  if( is_null(m_solution.lock()) )
    m_solution = find_component_ptr_with_tag<CField>( *(m_mesh.lock()) , "solution" );

  if( is_null(m_solution.lock()) )
    throw CastingFailed (FromHere(),
                         "Could not find a solution field on mesh "
                         + m_mesh.lock()->full_path().string() );

}

/////////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::execute()
{
  if (m_solution.expired())
    throw SetupError(FromHere(), "Mesh option in ["+full_path().path()+"] was not set");

  CField& field = *m_solution.lock();

  std::vector<Real> vars( DIM_3D, 0.);

  RealVector return_val( field.data().row_size() );

  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {
    boost_foreach(const Uint node, CElements::used_nodes(*region).array())
    {
      CTable<Real>::ConstRow coords = field.coords(node);

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

