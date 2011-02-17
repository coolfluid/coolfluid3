// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionT.hpp"
#include "Common/Log.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CFieldView.hpp"
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
      ->attach_trigger ( boost::bind ( &BcDirichlet::config_solution,   this ) )
      ->mark_basic()
      ->add_tag("solution");

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

void BcDirichlet::config_solution()
{
  URI uri;
  property("Solution").put_value(uri);

  Component::Ptr found = look_component(uri);
  if( is_null(found) )
    throw InvalidURI (FromHere(), "URI does not point to a component " + uri.string() );

  m_field = found->as_type<CField2>();
  if ( is_null( m_field.lock() ) )
    throw CastingFailed (FromHere(), "Solution field must be of a CField2 or derived type");
}

/////////////////////////////////////////////////////////////////////////////////////

void BcDirichlet::execute()
{
//  CFinfo << "boundary node [" << idx() << "]" << CFendl;


  // m_idx is the index that is set using the function set_loop_idx()
  CField2& field = *m_field.lock();
  CTable<Real>::Row data = field[idx()];

  Real vars[2];
  vars[XX] = field.coords(idx())[XX];
  vars[YY] = field.coords(idx())[YY];

//  CFinfo << "  --  coords " <<  x  << " " << y << CFendl;
//  CFinfo << "parsed function: " << func << CFendl;

  const Uint row_size = data.size();

  // Burgers - inlet bc:
  for (Uint i = 0; i != row_size; ++i)
  {
      data[i] = m_fparser.Eval(vars);
//      CFinfo << "data at x = " << vars[XX] << " = " << data[i] << CFendl;
  }
}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

