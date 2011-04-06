// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/mpl/for_each.hpp>

#include "Common/CBuilder.hpp"

#include "Common/Foreach.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/CRegion.hpp"

#include "Solver/CPhysicalModel.hpp"

    #include "RDM/CSysSUPG.hpp"

#include "RDM/SupportedTypes.hpp"    // supported elements

#include "RDM/LinearAdv2D.hpp"       // supported physics
#include "RDM/LinearAdvSys2D.hpp"    // supported physics
#include "RDM/RotationAdv2D.hpp"     // supported physics
#include "RDM/Burgers2D.hpp"         // supported physics
#include "RDM/Euler2D.hpp"           // supported physics

#include "RDM/SchemeCSysSUPG.hpp"

using namespace CF::Common;
using namespace CF::Mesh;
using namespace CF::Solver;

namespace CF {
namespace RDM {

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CSysSUPG, RDM::DomainTerm, LibRDM > CSysSUPG_Builder;

//////////////////////////////////////////////////////////////////////////////

CSysSUPG::CSysSUPG ( const std::string& name ) : RDM::DomainTerm(name)
{
  regist_typeinfo(this);
}

CSysSUPG::~CSysSUPG() {}

void CSysSUPG::execute()
{
  /// @todo physical model should be a configuration option of the solver
  CPhysicalModel::Ptr pm = find_component_ptr_recursively<CPhysicalModel>( *Core::instance().root() );
  if( is_null(pm) )
    throw ValueNotFound(FromHere(), "could not found any physical model to use");

  boost_foreach(Mesh::CRegion::Ptr& region, m_loop_regions)
  {
    std::string physics = pm->type();

    if ( physics == "LinearAdv2D" )
    {
      ElementLoop<CSysSUPG,LinearAdv2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }

    if ( physics == "LinearAdvSys2D" )
    {
      ElementLoop<CSysSUPG,LinearAdvSys2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }

    if ( physics == "RotationAdv2D" )
    {
      ElementLoop<CSysSUPG,RotationAdv2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }

    if ( physics == "Burgers2D" )
    {
      ElementLoop<CSysSUPG,Burgers2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }

    if ( physics == "Euler2D" )
    {
      ElementLoop<CSysSUPG,Euler2D> loop( *this, *region );
      boost::mpl::for_each< RDM::CellTypes >( loop );
    }

  }
}

//////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF
