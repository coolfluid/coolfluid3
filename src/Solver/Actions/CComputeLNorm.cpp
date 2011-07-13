// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include "Common/Log.hpp"

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CTable.hpp"

#include "Solver/Actions/CComputeLNorm.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {
namespace Actions {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < CComputeLNorm, CAction, LibActions > CComputeLNorm_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CComputeLNorm::CComputeLNorm ( const std::string& name ) : CAction(name)
{
  mark_basic();

  // properties

  m_properties.add_property("Norm", Real(0.) );

  // options

  m_options.add_option< OptionT<bool> >("Scale", true)
      ->set_description("Scales (divides) the norm by the number of entries (ignored if order zero)");

  m_options.add_option< OptionT<Uint> >("Order", 2u)
      ->set_description("Order of the p-norm, zero if L-inf");

  m_options.add_option(OptionComponent<CField>::create("Field", &m_field))
      ->set_description("Field for which to compute the norm");
}

////////////////////////////////////////////////////////////////////////////////

void CComputeLNorm::execute()
{
  if (m_field.expired()) throw SetupError(FromHere(), "Field was not set");

  CTable<Real>& table = m_field.lock()->data();
  CTable<Real>::ArrayT& array =  table.array();

  const Uint nbrows = table.size();

  if ( !nbrows ) throw SetupError(FromHere(), "Field has empty table");

  Real norm = 0.;

  const Uint order = m_options.option("Order").value<Uint>();
  switch(order)
  {
  case 2: // L2
    boost_foreach(CTable<Real>::ConstRow row, array )
        norm += row[0]*row[0];
    norm = std::sqrt(norm);
    break;
  case 1: // L1
    boost_foreach(CTable<Real>::ConstRow row, array )
        norm += std::abs( row[0] );
    break;
  case 0: // treat as Linf
    boost_foreach(CTable<Real>::ConstRow row, array )
        norm += std::max( std::abs(row[0]), norm );
    break;
  default: // Lp
    boost_foreach(CTable<Real>::ConstRow row, array )
        norm += std::abs( pow(row[0], order) );
    norm = std::pow(norm, 1./order );
    break;
  }

  if( m_options.option("Scale").value<bool>() && order )
    norm /= nbrows;

  configure_property("Norm", norm);
}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////

