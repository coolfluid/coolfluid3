// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/CBuilder.hpp"
#include "common/OptionURI.hpp"
#include "common/OptionArray.hpp"
#include "common/Log.hpp"
#include "common/FindComponents.hpp"

#include "Mesh/Geometry.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CList.hpp"

#include "RDM/BcDirichlet.hpp"
#include "RDM/RDSolver.hpp"

using namespace cf3::common;
using namespace cf3::Mesh;

namespace cf3 {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BcDirichlet, RDM::BoundaryTerm, LibRDM > BcDirichlet_Builder;

///////////////////////////////////////////////////////////////////////////////////////

BcDirichlet::BcDirichlet ( const std::string& name ) :
  RDM::BoundaryTerm(name)
{
  // options

  m_options.add_option< OptionArrayT<std::string> > ("functions", std::vector<std::string>())
      ->description("Math function applied as Dirichlet boundary condition (vars x,y)")
      ->attach_trigger ( boost::bind ( &BcDirichlet::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");
}


void BcDirichlet::config_function()
{
  std::vector<std::string> vs = m_options["functions"].value<std::vector<std::string> >();

  m_function.functions( vs );
  m_function.parse();
}


void BcDirichlet::execute()
{
  // ensure that the fields are present

  link_fields();

  // apply BC to solution field

  Field& solution_field = solution();

//  std::cout << "   field.size() == " << field.size() << std::endl;
//  std::cout << "   coordinates.size() == " << mesh().geometry().coordinates().size() << std::endl;

  std::vector<Real> vars( DIM_3D, 0.);

  RealVector return_val( solution_field.row_size() );

  boost_foreach(CRegion::Ptr& region, m_loop_regions)
  {

    /// @warning BcDirichlet assumes that solution maps one to one with mesh.geometry()

    Geometry& nodes = mesh().geometry();

//    std::cout << PERank << "  region \'" << region->uri().string() << "\'" << std::endl;
    boost_foreach(const Uint node, CElements::used_nodes(*region).array())
    {
      cf3_assert(node < solution_field.size());

      CTable<Real>::ConstRow coords = nodes.coordinates()[node];

      for (Uint i=0; i<coords.size(); ++i)
        vars[i] = coords[i];

      m_function.evaluate(vars,return_val);

      CTable<Real>::Row data_row = solution_field[node];
      for (Uint i=0; i<data_row.size(); ++i)
        data_row[i] = return_val[i];
    }

  }

}

////////////////////////////////////////////////////////////////////////////////////

} // RDM
} // cf3
