// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/mpl/for_each.hpp>
#include <boost/timer.hpp>


#include "common/Builder.hpp"

#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Region.hpp"

#include "Physics/PhysModel.hpp"

#include "RDM/SupportedCells.hpp"    // supported elements

#include "Physics/Scalar/LinearAdv2D.hpp"       // supported physics
#include "Physics/Scalar/RotationAdv2D.hpp"     // supported physics
#include "Physics/Scalar/Burgers2D.hpp"         // supported physics

#include "RDM/GPU/LDAGPU.hpp"
#include "RDM/GPU/SchemeLDAGPU.hpp"

using namespace cf3::common;
using namespace cf3::mesh;
using namespace cf3::Solver;

namespace cf3 {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < LDAGPU, RDM::CellTerm, LibGPU > LDAGPU_Builder;

////////////////////////////////////////////////////////////////////////////////

/// Looper defines a functor taking the type that boost::mpl::for_each passes.
/// It is the core of the looping mechanism.
template < typename PHYS>
struct LDAGPU::ElementLoop
{
  /// region to loop on
  mesh::Region& region;
  /// component containing the element loop
  LDAGPU& comp;
  /// Constructor
  ElementLoop( LDAGPU& comp_in, mesh::Region& region_in ) : comp(comp_in), region(region_in) {}
  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& T )
  {
    /// definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    /// parametrization of the numerical scheme
    typedef SchemeLDAGPU< SF, QD, PHYS > SchemeT;

    boost_foreach(mesh::Elements& elements,
                  common::find_components_recursively_with_filter<mesh::Elements>(region,IsElementType<SF>()))
    {
      // get the scheme or create it if does not exist
      Component::Ptr cscheme = comp.get_child_ptr( SchemeT::type_name() );
      typename SchemeT::Ptr scheme;
      if( is_null( cscheme ) )
        scheme = comp.create_component_ptr< SchemeT >( SchemeT::type_name() );
      else
        scheme = cscheme->as_ptr_checked<SchemeT>();

      // loop on elements of that type
      scheme->set_elements(elements);

      const Uint nb_elem = elements.size();
//      boost::timer ctimer;
      scheme->execute();
//      std::cout<<ctimer.elapsed()<<std::endl;
    }
  }

};

//////////////////////////////////////////////////////////////////////////////

LDAGPU::LDAGPU ( const std::string& name ) : RDM::CellTerm(name)
{
  regist_typeinfo(this);
}

LDAGPU::~LDAGPU() {}

void LDAGPU::execute()
{
  /// @todo physical model should be a configuration option of the solver
  Physics::PhysModel::Ptr pm = find_component_ptr_recursively<Physics::PhysModel>( Core::instance().root() );
  if( is_null(pm) )
    throw ValueNotFound(FromHere(), "could not found any physical model to use");

  boost_foreach(mesh::Region::Ptr& region, m_loop_regions)
  {
    std::string physics = pm->type();

    if ( physics == "LinearAdv2D" )
    {

      LDAGPU::ElementLoop< Physics::Scalar::LinearAdv2D > loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes2D >( loop );
    }

    if ( physics == "RotationAdv2D" )
    {
      LDAGPU::ElementLoop< Physics::Scalar::RotationAdv2D > loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes2D >( loop );
    }

    if ( physics == "Burgers2D" )
    {
      LDAGPU::ElementLoop< Physics::Scalar::Burgers2D > loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes2D >( loop );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
