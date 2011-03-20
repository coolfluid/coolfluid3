// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CList.hpp"

#include "RDM/BcDirichlet.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BcDirichlet, RDM::BoundaryTerm, LibRDM > BcDirichlet_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
BcDirichlet::BcDirichlet ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  // options

  m_properties.add_option< OptionURI > ("Solution",
                                        "Solution field where to apply the boundary condition",
                                        URI("cpath:"))
       ->attach_trigger ( boost::bind ( &BcDirichlet::config_mesh,   this ) )
       ->mark_basic()
       ->add_tag("solution");

  m_properties["Mesh"].as_option().attach_trigger ( boost::bind ( &BcDirichlet::config_mesh, this ) );

  m_properties.add_option<
      OptionT<std::string> > ("Function",
                              "Math function applied as Dirichlet boundary condition (vars x,y)",
                              "0.")
      ->attach_trigger ( boost::bind ( &BcDirichlet::config_function, this ) )
      ->mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::config_function()
{
  std::string func = m_properties["Function"].value<std::string>();
  int res = m_fparser.Parse(func, "x,y");

  if(res > 0)
    throw ParsingFailed(FromHere(),
                        "Parsing of math function failed with error \'"
                        + std::string(m_fparser.ErrorMsg())
                        + "\'" );
}

////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::config_mesh()
{
  cf_assert( is_not_null( m_mesh.lock() ) );

  URI sol_uri  = property("Solution").value<URI>();
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
  CField& field = *m_solution.lock();

  Real vars[DIM_2D];

  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {
    boost_foreach(const Uint node, CElements::used_nodes(*region).array())
    {
      CTable<Real>::Row data = field[node];

      vars[XX] = field.coords(node)[XX];
      vars[YY] = field.coords(node)[YY];

      const Uint row_size = data.size();
      for (Uint i = 0; i != row_size; ++i)
        data[i] = m_fparser.Eval(vars);

    }
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

