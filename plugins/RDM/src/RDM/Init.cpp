// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionArray.hpp"
#include "common/OptionComponent.hpp"
#include "common/FindComponents.hpp"

#include "mesh/SpaceFields.hpp"
#include "mesh/Region.hpp"
#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Elements.hpp"
#include "common/List.hpp"

#include "RDM/Init.hpp"
#include "RDM/RDSolver.hpp"

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Init, common::Action, LibRDM > Init_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

Init::Init ( const std::string& name ) :
  cf3::solver::Action(name)
{
  mark_basic();

  options().add_option(OptionComponent<Field>::create( "field", &m_field ))
      ->pretty_name("Solution Field")
      ->description("The field to Initialize");

  // options

  options().add_option< OptionArrayT<std::string> > ("functions", std::vector<std::string>())
      ->description("math function applied as Dirichlet boundary condition (vars x,y)")
      ->attach_trigger ( boost::bind ( &Init::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");
}


void Init::config_function()
{
  std::vector<std::string> vs = options()["functions"].value<std::vector<std::string> >();

  m_function.functions( vs );

  m_function.parse();
}


void Init::execute()
{
  if( is_null( m_field.lock() ) )
    m_field = solver().as_type<RDM::RDSolver>().fields()
        .get_child( RDM::Tags::solution() ).as_ptr_checked<Field>();

  Field& field = *m_field.lock();

  //  std::cout << "   field.size() == " << field.size() << std::endl;
  //  std::cout << "   coordinates.size() == " << mesh().geometry_fields().coordinates().size() << std::endl;

  std::vector<Real> vars( DIM_3D, 0.);

  RealVector return_val( field.row_size() );

  boost_foreach(Region::Ptr& region, m_loop_regions)
  {
    /// @warning assumes that field maps one to one with mesh.geometry_fields()

    SpaceFields& nodes = mesh().geometry_fields();

    boost_foreach(const Uint node, Elements::used_nodes(*region).array())
    {
      cf3_assert(node < field.size());

      Table<Real>::ConstRow coords = nodes.coordinates()[node];

      for (Uint i=0; i<coords.size(); ++i)
        vars[i] = coords[i];

      m_function.evaluate(vars,return_val);

      Table<Real>::Row data_row = field[node];
      for (Uint i=0; i<data_row.size(); ++i)
        data_row[i] = return_val[i];
    }

  }

}

////////////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
