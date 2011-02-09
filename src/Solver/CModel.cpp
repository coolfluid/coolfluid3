// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Solver/CModel.hpp"

#include "Mesh/CDomain.hpp"

namespace CF {
namespace Solver {

using namespace Common;
using namespace Mesh;

////////////////////////////////////////////////////////////////////////////////

CModel::CModel( const std::string& name  ) :
  Component ( name )
{
   mark_basic();

   properties()["steady"] = bool(true);

   // signals

   this->regist_signal ( "simulate" , "Simulates this model", "Simulate" )
       ->connect ( boost::bind ( &CModel::signal_simulate, this, _1 ) );

   m_domain = create_static_component<CDomain>("domain");
}

////////////////////////////////////////////////////////////////////////////////

CModel::~CModel()
{
}

////////////////////////////////////////////////////////////////////////////////

void CModel::signal_simulate ( Common::XmlNode& node )
{
  // XmlParams p ( node );

  this->simulate(); // dispatch to the virtual function
}

////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF
