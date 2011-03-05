// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/Blended.hpp"

// supported physics

#include "RDM/LinearAdv2D.hpp"
#include "RDM/RotationAdv2D.hpp"
#include "RDM/Burgers2D.hpp"

using namespace CF::Common;

namespace CF {
namespace RDM {

Common::ComponentBuilder < Blended<LinearAdv2D>,   CAction, LibRDM > Blended_LinearAdv2D_Builder;
Common::ComponentBuilder < Blended<RotationAdv2D>, CAction, LibRDM > Blended_RotationAdv2D_Builder;
Common::ComponentBuilder < Blended<Burgers2D>,     CAction, LibRDM > Blended_Burgers2D_Builder;

////////////////////////////////////////////////////////////////////////////////

template < typename PHYS >
Blended<PHYS>::Blended ( const std::string& name  ): RDM::Action(name)
{
  regist_typeinfo(this);
}

template < typename PHYS >
Blended<PHYS>::~Blended() {}

template < typename PHYS >
void Blended<PHYS>::execute()
{
  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {
    ElementLoop loop( *this, *region );

    boost::mpl::for_each< RDM::CellTypes >( loop );
  }
}

//////////////////////////////////////////////////////////////////////////////

/// Operator needed for the loop over element types, identified by shape functions (SF)
template < typename PHYS>
template < typename SF >
void Blended<PHYS>::ElementLoop::operator() ( SF& T )
{
  typedef typename RDM::DefaultQuadrature<SF>::type QD; // create a quadrature for this specific type
  typedef CSchemeB< SF, QD, PHYS > SchemeT;          // create a scheme for this specific type

  boost_foreach(Mesh::CElements& elements,
                Common::find_components_recursively_with_filter<Mesh::CElements>(region,IsElementType<SF>()))
  {
    // get the scheme
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

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
