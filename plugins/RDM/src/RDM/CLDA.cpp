// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"

#include "RDM/CLDA.hpp"

// supported physics

#include "RDM/LinearAdv2D.hpp"
#include "RDM/RotationAdv2D.hpp"
#include "RDM/Burgers2D.hpp"

using namespace CF::Common;

namespace CF {
namespace RDM {

Common::ComponentBuilder < CLDA<LinearAdv2D>,   CAction, LibRDM > CLDA_LinearAdv2D_Builder;
Common::ComponentBuilder < CLDA<RotationAdv2D>, CAction, LibRDM > CLDA_RotationAdv2D_Builder;
Common::ComponentBuilder < CLDA<Burgers2D>,     CAction, LibRDM > CLDA_Burgers2D_Builder;

////////////////////////////////////////////////////////////////////////////////

template < typename PHYS >
CLDA<PHYS>::CLDA ( const std::string& name  ): RDM::Action(name)
{
  regist_typeinfo(this);
}

template < typename PHYS >
CLDA<PHYS>::~CLDA() {}

template < typename PHYS >
void CLDA<PHYS>::execute()
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
void CLDA<PHYS>::ElementLoop::operator() ( SF& T )
{
  typedef typename RDM::DefaultQuadrature<SF>::type QD; // create a quadrature for this specific type
  typedef CSchemeLDAT< SF, QD, PHYS > SchemeT;          // create a scheme for this specific type

  boost_foreach(Mesh::CElements& elements,
                Common::find_components_recursively_with_filter<Mesh::CElements>(region,IsElementType<SF>()))
  {
    // get the scheme
    typename SchemeT::Ptr scheme = comp.get_child<SchemeT>( SchemeT::type_name() );
    if( is_null(scheme) )
      scheme = comp.create_component< SchemeT >( SchemeT::type_name() );

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
