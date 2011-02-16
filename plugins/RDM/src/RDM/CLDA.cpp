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
CLDA<PHYS>::CLDA ( const std::string& name  ): CLoop(name)
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
    CLDA::ElementLoop loop( *this, *region );

    boost::mpl::for_each< RDM::CellTypes >( loop );
  }
}

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
