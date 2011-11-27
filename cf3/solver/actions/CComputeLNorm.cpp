// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include "common/PE/Comm.hpp"

#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Foreach.hpp"

#include "mesh/Field.hpp"
#include "common/Table.hpp"

#include "solver/actions/CComputeLNorm.hpp"


using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////////////////

void compute_L2( Table<Real>::ArrayT& array, Real& norm )
{
  const int size = 1; // sum 1 value in each processor

  Real loc_norm = 0.; // norm on local processor
  Real glb_norm = 0.; // norm summed over all processors

  boost_foreach(Table<Real>::ConstRow row, array )
      loc_norm += row[0]*row[0];

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm, size, &glb_norm );

  norm = std::sqrt(glb_norm);
}

void compute_L1( Table<Real>::ArrayT& array, Real& norm )
{
  const int size = 1; // sum 1 value in each processor

  Real loc_norm = 0.; // norm on local processor
  Real glb_norm = 0.; // norm summed over all processors

  boost_foreach(Table<Real>::ConstRow row, array )
      loc_norm += std::abs( row[0] );

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm, size, &glb_norm );
}

void compute_Linf( Table<Real>::ArrayT& array, Real& norm )
{
  const int size = 1; // sum 1 value in each processor

  Real loc_norm = 0.; // norm on local processor
  Real glb_norm = 0.; // norm summed over all processors

  boost_foreach(Table<Real>::ConstRow row, array )
      loc_norm = std::max( std::abs(row[0]), loc_norm );

  PE::Comm::instance().all_reduce( PE::max(), &loc_norm, size, &glb_norm );

  norm = glb_norm;
}

void compute_Lp( Table<Real>::ArrayT& array, Real& norm, Uint order )
{
  const int size = 1; // sum 1 value in each processor

  Real loc_norm = 0.; // norm on local processor
  Real glb_norm = 0.; // norm summed over all processors

  boost_foreach(Table<Real>::ConstRow row, array )
    loc_norm += std::pow( std::abs(row[0]), (int)order ) ;

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm, size, &glb_norm );

  norm = std::pow(glb_norm, 1./order );
}

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CComputeLNorm, Action, LibActions > CComputeLNorm_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CComputeLNorm::CComputeLNorm ( const std::string& name ) : Action(name)
{
  mark_basic();

  // properties

  properties().add_property("norm", Real(0.) );

  // options

  options().add_option("Scale", true)
      .description("Scales (divides) the norm by the number of entries (ignored if order zero)");

  options().add_option("Order", 2u)
      .description("Order of the p-norm, zero if L-inf");

  options().add_option("Field", m_field)
      .description("Field for which to compute the norm")
      .link_to(&m_field);
}


void CComputeLNorm::execute()
{
  if ( is_null(m_field) ) 	throw SetupError(FromHere(), "Field was not set");

  Table<Real>& table = *m_field;
  Table<Real>::ArrayT& array =  m_field->array();

  const Uint nbrows = table.size();

  if ( !nbrows ) throw SetupError(FromHere(), "Field has empty table");

  Real norm = 0.;

  const Uint order = options().option("Order").value<Uint>();

  // sum of all processors

  switch(order) {

  case 2:  compute_L2( array, norm );    break;

  case 1:  compute_L1( array, norm );    break;

  case 0:  compute_Linf( array, norm );  break; // consider order 0 as Linf

  default: compute_Lp( array, norm, order );    break;

  }


  if( options().option("Scale").value<bool>() && order )
    norm /= nbrows;

  properties().configure_property("norm", norm);
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
