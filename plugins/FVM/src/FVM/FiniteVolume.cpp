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
#include "Common/OptionURI.hpp"

#include "Mesh/CField2.hpp"
#include "Mesh/CRegion.hpp"

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

  properties()["brief"] = std::string("Finite Volume Method");
  properties()["description"] = std::string("Discretize the PDE's using the Cell Centered Finite Volume Method");
    
  m_properties.add_option< OptionURI >("Solution","Solution Field to discretize",URI("cpath:"));
  m_properties.add_option< OptionURI >("Residual","Residual Field to compute",URI("cpath:"));
  m_properties.add_option< OptionURI >("UpdateCoeff","UpdateCoeff to compute",URI("cpath:"));
  m_properties["Solution"].as_option().attach_trigger ( boost::bind ( &FiniteVolume::configure_solution,   this ) );
  m_properties["Residual"].as_option().attach_trigger ( boost::bind ( &FiniteVolume::configure_residual,   this ) );
  m_properties["UpdateCoeff"].as_option().attach_trigger ( boost::bind ( &FiniteVolume::configure_update_coeff,   this ) );
  
  m_solution = create_static_component<CLink>("solution");
  m_residual = create_static_component<CLink>("residual");
  m_update_coeff = create_static_component<CLink>("update_coeff");
  
  // setup of the static components
  CF_DEBUG_POINT;
  m_face_loop = create_static_component<CForAllFaces>("for_all_faces");
  m_face_loop->create_action("CF.FVM.ComputeFlux");  
}

////////////////////////////////////////////////////////////////////////////////

FiniteVolume::~FiniteVolume()
{
}

//////////////////////////////////////////////////////////////////////////////

void FiniteVolume::configure_solution()
{
  URI uri; property("Solution").put_value(uri);
  CField2::Ptr field = Core::instance().root()->look_component<CField2>(uri);
  m_solution->link_to(field);
  m_face_loop->action("CF.FVM.ComputeFlux").configure_property("Solution",field->full_path());  
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolume::configure_residual()
{
  URI uri; property("Residual").put_value(uri);
  CField2::Ptr field = Core::instance().root()->look_component<CField2>(uri);
  m_residual->link_to(field);
  
  CFinfo << "face_loop configuration" << CFendl;
  std::vector<URI> faces_to_loop;
  faces_to_loop.push_back(field->topology().full_path());
  m_face_loop->configure_property("Regions" , faces_to_loop);
  m_face_loop->action("CF.FVM.ComputeFlux").configure_property("Residual",field->full_path());  
  
}

////////////////////////////////////////////////////////////////////////////////

void FiniteVolume::configure_update_coeff()
{
  URI uri; property("UpdateCoeff").put_value(uri);
  CField2::Ptr field = Core::instance().root()->look_component<CField2>(uri);
  m_update_coeff->link_to(field);
}

////////////////////////////////////////////////////////////////////////////////

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
