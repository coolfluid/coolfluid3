// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/Log.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/CField2.hpp"

#include "Solver/CIterativeSolver.hpp"
#include "Solver/Actions/CForAllFaces.hpp"

#include "FVM/FiniteVolume.hpp"

using namespace boost::assign;

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;
using namespace CF::Solver::Actions;

namespace CF {
namespace FVM {

Common::ComponentBuilder < FiniteVolume, CDiscretization, LibFVM > FiniteVolume_Builder;

////////////////////////////////////////////////////////////////////////////////

FiniteVolume::FiniteVolume ( const std::string& name  ) :
  CDiscretization ( name )
{
  // properties

  properties()["brief"] = std::string("Residual Distribution Method");
  properties()["description"] = std::string("Discretize the PDE's using the Residual Distribution Method");
  
  m_properties["Regions"].as_option().attach_trigger ( boost::bind ( &FiniteVolume::trigger_Regions,   this ) );
    
  // setup of the static components
  m_face_loop = create_static_component<CForAllFaces>("for_all_faces");
  m_face_loop->create_action("CF.Solver.Actions.CComputeVolume");
  // m_elem_loop = create_static_component<
  //     CForAllT< CSchemeLDAT< ShapeFunctionT, QuadratureT > , Mesh::SF::CellTypes > >("cell_loop");
  
}

////////////////////////////////////////////////////////////////////////////////

FiniteVolume::~FiniteVolume()
{
}

//////////////////////////////////////////////////////////////////////////////

void FiniteVolume::trigger_Regions()
{
  CFinfo << "face_loop configuration" << CFendl;
  m_face_loop->configure_property("Regions" , property("Regions").value<std::vector<URI> >());
  m_face_loop->action("CF.Solver.Actions.CComputeVolume").configure_property("Volumes",find_component_recursively_with_name<CField2>(*Core::instance().root(),"residual").full_path());
  
}
//////////////////////////////////////////////////////////////////////////////

void FiniteVolume::create_bc( XmlNode& xml )
{
  XmlParams p (xml);

  std::string name = p.get_option<std::string>("Name");
//   
//   CLoop::Ptr apply_bc = create_component< CForAllNodes >(name);
// 
//   apply_bc->create_action("CF.Solver.Actions.CSetFieldValues");
//   apply_bc->action("CF.Solver.Actions.CSetFieldValues").configure_property("Field",std::string("solution"));
//   apply_bc->add_tag("apply_bc_action");
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolume::compute_rhs()
{
  // apply BC
  boost_foreach (CLoop& apply_bc, find_components_with_tag<CLoop>(*this,"apply_bc_action"))
  {
    CFinfo << apply_bc.name() << CFendl;
    apply_bc.execute();
  }

  // compute element residual distribution
  CFinfo << "execute " << m_face_loop->name() << CFendl;
  m_face_loop->execute();
}

////////////////////////////////////////////////////////////////////////////////

} // FVM
} // CF
