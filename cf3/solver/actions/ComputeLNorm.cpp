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

  properties().add_property("norm", Real(0.) );

  // options

  options().add_option("scale", true)
      .description("Scales (divides) the norm by the number of entries (ignored if order zero)");

  options().add_option("order", 2u)
      .description("Order of the p-norm, zero if L-inf");

  options().add_option("field", URI())
      .pretty_name("Field")
      .description("URI to the field to use, or to a link");
  }

////////////////////////////////////////////////////////////////////////////////////////////

std::vector<Real> ComputeLNorm::compute_norm(mesh::Field& field) const
{

  const Uint loc_nb_rows = field.size(); // field size on local processor
  Uint nb_rows = 0;                      // field size over all processors

  PE::Comm::instance().all_reduce( PE::plus(), &loc_nb_rows, 1u, &nb_rows );

  if ( !nb_rows ) throw SetupError(FromHere(), "Field is empty");

  std::vector<Real> norms(field.row_size(), 0.);

  const Uint order = options().option("order").value<Uint>();

  // sum of all processors

  switch(order) {

  case 2:  compute_L2( field.array(), norms );    break;

  case 1:  compute_L1( field.array(), norms );    break;

  case 0:  compute_Linf( field.array(), norms );  break; // consider order 0 as Linf

  default: compute_Lp( field.array(), norms, order );    break;

  }

  if( options().option("scale").value<bool>() && order )
  {
    for (Uint i=0; i<norms.size(); ++i)
      norms[i] /= nb_rows;
  }

  return norms;
}

void ComputeLNorm::execute()
{
  Handle<Field> field( follow_link(access_component(options().option("field").value<URI>())) );
  if(is_not_null(field))
  {
    std::vector<Real> norms = compute_norm(*field);

    /// @todo this first one should dissapear
    properties().configure_property("norm", norms[0] );
    properties()["norms"] = norms;
  }
  else
    CFinfo << "Not computing norm in action " << uri() << " because option field is invalid." << CFendl;
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
