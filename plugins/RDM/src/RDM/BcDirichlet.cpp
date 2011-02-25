// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CField2.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CEntities.hpp"

#include "RDM/BcDirichlet.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < BcDirichlet, CAction, LibRDM > BcDirichlet_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
BcDirichlet::BcDirichlet ( const std::string& name ) :
  Solver::Actions::CLoopOperation(name)
{
  // options

  m_properties.add_option< OptionURI > ("Solution",
                                        "Solution field where to apply the boundary condition",
                                        URI("cpath:"))
       ->attach_trigger ( boost::bind ( &BcDirichlet::config_mesh,   this ) )
       ->mark_basic()
       ->add_tag("solution");

  m_properties.add_option< OptionURI > ("Mesh",
                                        "Mesh where the boundary exists",
                                        URI("cpath:"))
      ->attach_trigger ( boost::bind ( &BcDirichlet::config_mesh,   this ) )
      ->add_tag("mesh");

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
  URI mesh_uri = property("Mesh").value<URI>();

  Component::Ptr mesh_comp = access_component_ptr(mesh_uri);
  if( is_null(mesh_comp) )
    throw InvalidURI (FromHere(), "URI does not point to a component " + mesh_uri.string() );

  m_mesh = mesh_comp->as_ptr<CMesh>();
  if ( is_null( m_mesh.lock() ) )
    throw CastingFailed (FromHere(), "Could not find a CMesh on path " + mesh_uri.string() );

  URI sol_uri  = property("Solution").value<URI>();
  m_solution = access_component_ptr(sol_uri)->as_ptr<CField2>();
  if( is_null(m_solution.lock()) )
    m_solution = find_component_ptr_with_tag<CField2>( *(m_mesh.lock()) , "solution" );

  if( is_null(m_solution.lock()) )
    throw CastingFailed (FromHere(), "Could not find a solution field on mesh " + mesh_uri.string() );

}

/////////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::execute()
{
  // idx() is the node index

  CField2& field = *m_solution.lock();
  CTable<Real>::Row data = field[idx()];

  Real vars[2];
  vars[XX] = field.coords(idx())[XX];
  vars[YY] = field.coords(idx())[YY];

  const Uint row_size = data.size();

  for (Uint i = 0; i != row_size; ++i)
  {
      data[i] = m_fparser.Eval(vars);
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

