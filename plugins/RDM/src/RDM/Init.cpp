// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CNodes.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CList.hpp"

#include "RDM/Init.hpp"
#include "RDM/RDSolver.hpp"

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Init, CF::Solver::Action, LibRDM > Init_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Init::Init ( const std::string& name ) :
  CF::Solver::Action(name)
{
  mark_basic();

  m_options.add_option(OptionComponent<CField>::create( "field", &m_field ))
      ->pretty_name("Field")
      ->description("The field to Initialize");

  // options

  m_options.add_option< OptionArrayT<std::string> > ("functions", std::vector<std::string>())
      ->description("Math function applied as Dirichlet boundary condition (vars x,y)")
      ->attach_trigger ( boost::bind ( &Init::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");
}


void Init::config_function()
{
  m_function.functions( m_options["functions"].value<std::vector<std::string> >() );
  m_function.parse();
}


void Init::execute()
{
  if( is_null( m_field.lock() ) )
    m_field = solver().as_type<RDM::RDSolver>().fields()
        .get_child( RDM::Tags::solution() ).as_ptr_checked<CField>();

  CField& field = *m_field.lock();

  //  std::cout << "   field.size() == " << field.size() << std::endl;
  //  std::cout << "   coordinates.size() == " << mesh().nodes().coordinates().size() << std::endl;

  std::vector<Real> vars( DIM_3D, 0.);

  RealVector return_val( field.data().row_size() );

  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {
    /// @warning assumes that field maps one to one with mesh.nodes()

    CNodes& nodes = mesh().nodes();

    boost_foreach(const Uint node, CElements::used_nodes(*region).array())
    {
      cf_assert(node < field.size());

      CTable<Real>::ConstRow coords = nodes.coordinates()[node];

      for (Uint i=0; i<coords.size(); ++i)
        vars[i] = coords[i];

      m_function.evaluate(vars,return_val);

      CTable<Real>::Row data_row = field[node];
      for (Uint i=0; i<data_row.size(); ++i)
        data_row[i] = return_val[i];
    }

  }

}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
