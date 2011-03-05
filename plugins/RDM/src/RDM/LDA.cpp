// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/mpl/for_each.hpp>

#include "Common/CBuilder.hpp"

#include "Common/Foreach.hpp"
#include "Common/ComponentPredicates.hpp"

#include "RDM/LDA.hpp"

#include "RDM/SupportedTypes.hpp"    // supported elements

#include "RDM/LinearAdv2D.hpp"       // supported physics
#include "RDM/RotationAdv2D.hpp"     // supported physics
#include "RDM/Burgers2D.hpp"         // supported physics

#include "RDM/CSchemeLDAT.hpp"

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < LDA, CAction, LibRDM > LDA_Builder;

////////////////////////////////////////////////////////////////////////////////

/// Looper defines a functor taking the type that boost::mpl::for_each passes.
/// It is the core of the looping mechanism.
template < typename PHYS>
struct ElementLoop
{
  /// region to loop on
  Mesh::CRegion& region;
  /// component containing the element loop
  LDA& comp;
  /// Constructor
  ElementLoop( LDA& comp_in, Mesh::CRegion& region_in ) : comp(comp_in), region(region_in) {}
  /// operator needed for the loop over element types (SF)
  template < typename SF >
  void operator() ( SF& T )
  {
    /// definition of the quadrature type
    typedef typename RDM::DefaultQuadrature<SF>::type QD;
    /// parametrization of the numerical scheme
    typedef CSchemeLDAT< SF, QD, PHYS > SchemeT;

    boost_foreach(Mesh::CElements& elements,
                  Common::find_components_recursively_with_filter<Mesh::CElements>(region,IsElementType<SF>()))
    {
      // get the scheme or create it if does not exist
      Component::Ptr cscheme = comp.get_child_ptr( SchemeT::type_name() );
      typename SchemeT::Ptr scheme;
      if( is_null( cscheme ) )
        scheme = comp.create_component< SchemeT >( SchemeT::type_name() );
      else
        scheme = cscheme->as_ptr_checked<SchemeT>();

      // loop on elements of that type
      scheme->set_elements(elements);

      const Uint nb_elem = elements.size();
      for ( Uint elem = 0; elem != nb_elem; ++elem )
      {
        scheme->select_loop_idx(elem);
        scheme->execute();
      }
    }
  }

};

//////////////////////////////////////////////////////////////////////////////

LDA::LDA ( const std::string& name  ): RDM::Action(name)
{
  regist_typeinfo(this);
}

LDA::~LDA() {}

void LDA::execute()
{
  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {
    std::string physics;

    if ( physics == "LinearAdv2D" )
    {
      ElementLoop<LinearAdv2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }

    if ( physics == "RotationAdv2D" )
    {
      ElementLoop<RotationAdv2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }

    if ( physics == "Burgers2D" )
    {
      ElementLoop<Burgers2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }
  }
}

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
