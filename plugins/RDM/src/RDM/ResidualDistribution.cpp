// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"

#include "Solver/Actions/CLoop.hpp"
#include "Solver/Actions/CForAllT.hpp"
#include "Solver/Actions/CForAllNodes.hpp"

#include "RDM/ResidualDistribution.hpp"
#include "RDM/CSchemeLDAT.hpp"
#include "RDM/CSchemeN.hpp"

#include "Mesh/SF/Triag2DLagrangeP1.hpp"
#include "Mesh/SF/Triag2DLagrangeP2.hpp"
#include "Mesh/SF/Triag2DLagrangeP2B.hpp"
#include "Mesh/SF/Triag2DLagrangeP3.hpp"
#include "Mesh/SF/Quad2DLagrangeP1.hpp"
#include "Mesh/SF/Quad2DLagrangeP2.hpp"

#include "Mesh/Integrators/Gauss.hpp"

using namespace boost::assign;

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace RDM {

Common::ComponentBuilder < ResidualDistribution, CDiscretization, LibRDM > ResidualDistribution_Builder;

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::ResidualDistribution ( const std::string& name  ) :
  CDiscretization ( name )
{
  // properties

  properties()["brief"] = std::string("Residual Distribution Method");
  std::string desc =
        "Discretize the PDE's using the Cell Centered Finite Volume Method\n"
        "This method ... (explain)";
  properties()["description"] = desc ;


  // signals

  regist_signal ( "create_boundary_term" , "creates a boundary condition term", "Create Boundary Condition" )->connect ( boost::bind ( &ResidualDistribution::create_boundary_term, this, _1 ) );
  regist_signal ( "create_domain_term" , "creates a domain volume term", "Create Domain Term" )->connect ( boost::bind ( &ResidualDistribution::create_domain_term, this, _1 ) );
    
  // setup of the static components

  // create apply boundary conditions action
  m_compute_boundary_face_terms = create_static_component<CAction>("compute_boundary_faces");
  m_compute_boundary_face_terms->mark_basic();

  // create compute rhs action
  m_compute_volume_cell_terms = create_static_component<CAction>("compute_volume_cells");
  m_compute_volume_cell_terms->mark_basic();


#ifdef RDOLD
//--- move from here ---

  const Uint order = 5;

  //  typedef SF::Triag2DLagrangeP1 ShapeFunctionT;
   // typedef SF::Quad2DLagrangeP1  ShapeFunctionT;
   //   typedef SF::Triag2DLagrangeP2 ShapeFunctionT;
    typedef SF::Triag2DLagrangeP2B ShapeFunctionT;
  //  typedef SF::Triag2DLagrangeP3 ShapeFunctionT;
  //  typedef SF::Quad2DLagrangeP2  ShapeFunctionT;

  typedef Mesh::Integrators::GaussMappedCoords<order,ShapeFunctionT::shape> QuadratureT;

  m_elem_loop = create_static_component<
      CForAllT< CSchemeLDAT< ShapeFunctionT, QuadratureT > , Mesh::SF::CellTypes > >("cell_loop");
  
// ---------------------
#endif
}

////////////////////////////////////////////////////////////////////////////////

ResidualDistribution::~ResidualDistribution() {}

//////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::create_boundary_term( XmlNode& xml )
{
  XmlParams p (xml);

  std::string name = p.get_option<std::string>("Name");
  std::string type = p.get_option<std::string>("Type");

  std::vector<URI> regions = p.get_array<URI>("Regions");

  CAction& face_loop =
      m_compute_boundary_face_terms->create_action("CF.Solver.Actions.CForAllFaces", name);

  face_loop.configure_property("Regions" , regions);
  face_loop.mark_basic();

  CAction& face_action = face_loop.create_action( type , "action" );
  face_action.mark_basic();
}

//////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::create_domain_term( XmlNode& xml )
{
  XmlParams p (xml);

  std::string name = p.get_option<std::string>("Name");
  std::string type = p.get_option<std::string>("Type");

  std::vector<URI> regions = p.get_array<URI>("Regions");

  CAction& cell_loop = m_compute_volume_cell_terms->create_action("CF.RDM.CForAllCells", name);

  cell_loop.configure_property("Regions" , regions);
  cell_loop.mark_basic();

  CAction& cell_action = cell_loop.create_action( type , "action" );
  cell_action.mark_basic();
}

////////////////////////////////////////////////////////////////////////////////

void ResidualDistribution::compute_rhs()
{
  m_compute_boundary_face_terms->execute();
  m_compute_volume_cell_terms->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
