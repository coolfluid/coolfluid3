// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <cmath>

#include "common/PE/Comm.hpp"

#include "common/Builder.hpp"
#include "common/Log.hpp"
#include "common/OptionT.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"
#include "common/Foreach.hpp"

#include "mesh/Field.hpp"
#include "common/Table.hpp"

#include "solver/actions/ComputeLNorm.hpp"


using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

////////////////////////////////////////////////////////////////////////////////////////////

void compute_L2( Table<Real>::ArrayT& array, std::vector<Real>& norms )
{

  std::vector<Real> loc_norm(norms.size(),0.); // norm on local processor
  std::vector<Real> glb_norm(norms.size(),0.); // norm summed over all processors

  boost_foreach(Table<Real>::ConstRow row, array )
  {
    for (Uint i=0; i<norms.size(); ++i)
      loc_norm[i] += row[i]*row[i];
  }

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm[0], norms.size(), &glb_norm[0] );

  for (Uint i=0; i<norms.size(); ++i)
    norms[i] = std::sqrt(glb_norm[i]);
}

void compute_L1( Table<Real>::ArrayT& array, std::vector<Real>& norms )
{
  std::vector<Real> loc_norm(norms.size(),0.); // norm on local processor

  boost_foreach(Table<Real>::ConstRow row, array )
  {
    for (Uint i=0; i<norms.size(); ++i)
      loc_norm[i] += std::abs( row[i] );
  }

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm[0], norms.size(), &norms[0] );
}

void compute_Linf( Table<Real>::ArrayT& array, std::vector<Real>& norms )
{
  std::vector<Real> loc_norm(norms.size(),0.); // norm on local processor

  boost_foreach(Table<Real>::ConstRow row, array )
  {
    for (Uint i=0; i<norms.size(); ++i)
      loc_norm[i] = std::max( std::abs(row[i]), loc_norm[i] );
  }

  PE::Comm::instance().all_reduce( PE::max(), &loc_norm[0], norms.size(), &norms[0] );
}

void compute_Lp( Table<Real>::ArrayT& array, std::vector<Real>& norms, Uint order )
{
  const int size = 1; // sum 1 value in each processor

  std::vector<Real> loc_norm(norms.size(),0.); // norm on local processor
  std::vector<Real> glb_norm(norms.size(),0.); // norm summed over all processors

  boost_foreach(Table<Real>::ConstRow row, array )
  {
    for (Uint i=0; i<norms.size(); ++i)
      loc_norm[i] += std::pow( std::abs(row[i]), (int)order ) ;
  }

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm[0], norms.size(), &glb_norm[0] );

  for (Uint i=0; i<norms.size(); ++i)
    norms[i] = std::pow(glb_norm[i], 1./order );
}

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeLNorm, Action, LibActions > ComputeLNorm_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ComputeLNorm::ComputeLNorm ( const std::string& name ) : Action(name)
{
  mark_basic();

  // properties

  properties().add("norm", Real(0.) );

  // options

  options().add("scale", true)
      .description("Scales (divides) the norm by the number of entries (ignored if order zero)");

  options().add("order", 2u)
      .description("Order of the p-norm, zero if L-inf");

  options().add("table", URI())
      .pretty_name("Table")
      .description("URI to the table to use, or to a link");
  }

////////////////////////////////////////////////////////////////////////////////////////////

std::vector<Real> ComputeLNorm::compute_norm(Table<Real>& table) const
{

  const Uint loc_nb_rows = table.size(); // table size on local processor
  Uint nb_rows = 0;                      // table size over all processors

  PE::Comm::instance().all_reduce( PE::plus(), &loc_nb_rows, 1u, &nb_rows );

  if ( !nb_rows ) throw SetupError(FromHere(), "Table is empty");

  std::vector<Real> norms(table.row_size(), 0.);

  const Uint order = options().value<Uint>("order");

  // sum of all processors

  switch(order) {

  case 2:  compute_L2( table.array(), norms );    break;

  case 1:  compute_L1( table.array(), norms );    break;

  case 0:  compute_Linf( table.array(), norms );  break; // consider order 0 as Linf

  default: compute_Lp( table.array(), norms, order );    break;

  }

  if( options().value<bool>("scale") && order )
  {
    for (Uint i=0; i<norms.size(); ++i)
      norms[i] /= nb_rows;
  }

  return norms;
}

void ComputeLNorm::execute()
{
  Handle< Table<Real> > table( follow_link(access_component(options().value<URI>("table"))) );
  if(is_not_null(table))
  {
    std::vector<Real> norms = compute_norm(*table);

    /// @todo this first one should dissapear
    properties().set("norm", norms[0] );
    properties()["norms"] = norms;
  }
  else
    CFinfo << "Not computing norm in action " << uri() << " because option table is invalid." << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
