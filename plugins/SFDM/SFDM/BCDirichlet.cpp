// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "common/Log.hpp"
#include "common/Builder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionArray.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Field.hpp"
#include "mesh/SpaceFields.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Space.hpp"
#include "mesh/ElementType.hpp"
#include "mesh/Region.hpp"
#include "mesh/Cells.hpp"
#include "mesh/Faces.hpp"
#include "mesh/CellFaces.hpp"
#include "mesh/UnifiedData.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/FaceCellConnectivity.hpp"
#include "mesh/ElementConnectivity.hpp"

#include "RiemannSolvers/RiemannSolvers/RiemannSolver.hpp"

#include "physics/Variables.hpp"
#include "physics/PhysModel.hpp"


#include "SFDM/BCDirichlet.hpp"
#include "SFDM/ShapeFunction.hpp"
#include "SFDM/Tags.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

  using namespace common;
  using namespace mesh;
  using namespace physics;
  using namespace RiemannSolvers;


////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < BCDirichlet, Term, LibSFDM> BCDirichlet_builder;

//////////////////////////////////////////////////////////////////////////////

BCDirichlet::BCDirichlet( const std::string& name )
  : Term(name)
{
  properties()["brief"] = std::string("Convective Spectral Finite Difference term");
  properties()["description"] = std::string("Fields to be created: ...");


  options().add_option(OptionComponent<Variables>::create( "input_vars", &m_input_vars))
      ->pretty_name("Input Variables")
      ->description("The input variables.\nIf empty, Solution Variables will be used");

  options().add_option< OptionArrayT<std::string> > ("functions", std::vector<std::string>())
      ->pretty_name("Functions")
      ->description("math function applied as initial condition using Input Variables (vars x,y)")
      ->attach_trigger ( boost::bind ( &BCDirichlet::config_function, this ) )
      ->mark_basic();

  m_function.variables("x,y,z");
}


/////////////////////////////////////////////////////////////////////////////

void BCDirichlet::config_function()
{
  std::vector<std::string> vs = options()["functions"].value<std::vector<std::string> >();

  m_function.functions( vs );

  m_function.parse();
}

/////////////////////////////////////////////////////////////////////////////

void BCDirichlet::execute()
{
  link_fields();

  if (m_input_vars.expired())
    m_input_vars = m_solution_vars.lock();
  Variables& input_vars    = *m_input_vars.lock();
  const Uint nb_vars = physical_model().neqs();

  sol_in_flx_pt.resize(physical_model().neqs());
  flx_in_flx_pt.resize(physical_model().neqs());

  RealVector params(3); params.setZero();
  RealVector sol_in_bc(nb_vars);

  /// Faces loop
  boost_foreach(Region::Ptr region, m_loop_regions)
  boost_foreach(Faces& faces, find_components_recursively<Faces>(*region))
  {
    FaceCellConnectivity& cell_connectivity = faces.get_child("cell_connectivity").as_type<FaceCellConnectivity>();

    /// For every face
    for (Face2Cell face(cell_connectivity); face.idx<cell_connectivity.size(); ++face.idx)
    {
      Flyweight fly = create_flyweight(face.cells()[LEFT]);
      const Uint nb_face_pts = fly.sf.face_flx_pts(face.face_nb_in_cells()[LEFT]).size();
      const Uint face_nb = face.face_nb_in_cells()[LEFT];

      /// For every face flux point
      boost_foreach(const Uint flx_pt, fly.sf.face_flx_pts(face_nb))
      {
        /// 1) compute solution in face flux point
        fly.reconstruct_solution_in_flx_pt(flx_pt,sol_in_flx_pt);

        /// 2) get solution from boundary condition
        // Set function parameters
        /// @todo use interpolation from geometry_nodes
        for (Uint d=0; d<cache.geometry_nodes.cols(); ++d)
          params[d] = cache.geometry_nodes(0,d);

        // Evaluate function
        m_function.evaluate(params,sol_in_bc);

        // Transform the return_val of the function to properties
        input_vars.compute_properties(cache.geometry_nodes.row(0),sol_in_bc,cache.dummy_grads,*cache.phys_props);

        // compute solution from properties
        solution_vars().compute_variables(*cache.phys_props,sol_in_bc);


        /// 3) compute numerical flux in this point
        fly.compute_numerical_flux(flx_pt,sol_in_flx_pt,sol_in_bc,
                                   flx_in_flx_pt,wave_speed_in_flx_pt);

        std::cout << "face for cell " << face.cells()[LEFT] << std::endl;
        std::cout << "   inside  = " << sol_in_flx_pt << std::endl;
        std::cout << "   outside = " << sol_in_bc << std::endl;
        std::cout << "   flux = " << flx_in_flx_pt << std::endl;

        /// 3) add contribution of this flux-point to solution points for the
        ///    gradient of the flux
        fly.add_flx_pt_gradient_contribution_to_residual(flx_pt,flx_in_flx_pt);

        /// 5) add contribution of this flux-point to solution points for the
        ///    interpolation of the wave_speed
        fly.add_flx_pt_contribution_to_wave_speed(flx_pt,wave_speed_in_flx_pt);
      }
      // end face_pt loop
    }
    // end face loop
  }
  // end boundary faces loop
}

/////////////////////////////////////////////////////////////////////////////

} // SFDM
} // cf3
