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
#include "mesh/Space.hpp"
#include "mesh/Connectivity.hpp"

#include "sdm/ComputeLNorm.hpp"
#include "solver/History.hpp"

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace sdm {

////////////////////////////////////////////////////////////////////////////////////////////

void compute_L2( const Field& field, std::vector<Real>& norms )
{

  std::vector<Real> loc_norm(norms.size(),0.); // norm on local processor
  std::vector<Real> glb_norm(norms.size(),0.); // norm summed over all processors

  // loop over all elements
  boost_foreach (const Handle<Space>& space, field.spaces() )
  {
    // only if the elements are volume elements
    if (space->support().element_type().dimension() == space->support().element_type().dimensionality())
    {
      for (Uint e=0; e<space->size(); ++e)
      {
        if (!space->support().is_ghost(e))
        {
          // compute norm for these nodes
          boost_foreach( const Uint node, space->connectivity()[e] )
          {
            for (Uint i=0; i<norms.size(); ++i)
              loc_norm[i] += field[node][i]*field[node][i];
          }
        }
      }
    }
  }

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm[0], norms.size(), &glb_norm[0] );

  for (Uint i=0; i<norms.size(); ++i)
    norms[i] = std::sqrt(glb_norm[i]);
}

////////////////////////////////////////////////////////////////////////////////

void compute_L1( const Field& field, std::vector<Real>& norms )
{
  std::vector<Real> loc_norm(norms.size(),0.); // norm on local processor

  // loop over all elements
  boost_foreach (const Handle<Space>& space, field.spaces() )
  {
    // only if the elements are volume elements
    if (space->support().element_type().dimension() == space->support().element_type().dimensionality())
    {
      for (Uint e=0; e<space->size(); ++e)
      {
        if (!space->support().is_ghost(e))
        {
          // compute norm for these nodes
          boost_foreach( const Uint node, space->connectivity()[e] )
          {
            for (Uint i=0; i<norms.size(); ++i)
              loc_norm[i] += std::abs( field[node][i] );
          }
        }
      }
    }
  }

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm[0], norms.size(), &norms[0] );
}

////////////////////////////////////////////////////////////////////////////////

void compute_Linf( const Field& field, std::vector<Real>& norms )
{
  std::vector<Real> loc_norm(norms.size(),0.); // norm on local processor

  // loop over all elements
  boost_foreach (const Handle<Space>& space, field.spaces() )
  {
    // only if the elements are volume elements
    if (space->support().element_type().dimension() == space->support().element_type().dimensionality())
    {
      for (Uint e=0; e<space->size(); ++e)
      {
        if (!space->support().is_ghost(e))
        {
          // compute norm for these nodes
          boost_foreach( const Uint node, space->connectivity()[e] )
          {
            for (Uint i=0; i<norms.size(); ++i)
              loc_norm[i] = std::max( std::abs(field[node][i]), loc_norm[i] );
          }
        }
      }
    }
  }

  PE::Comm::instance().all_reduce( PE::max(), &loc_norm[0], norms.size(), &norms[0] );
}

////////////////////////////////////////////////////////////////////////////////

void compute_Lp( const Field& field, std::vector<Real>& norms, Uint order )
{
  const int size = 1; // sum 1 value in each processor

  std::vector<Real> loc_norm(norms.size(),0.); // norm on local processor
  std::vector<Real> glb_norm(norms.size(),0.); // norm summed over all processors

  // loop over all elements
  boost_foreach (const Handle<Space>& space, field.spaces() )
  {
    // only if the elements are volume elements
    if (space->support().element_type().dimension() == space->support().element_type().dimensionality())
    {
      for (Uint e=0; e<space->size(); ++e)
      {
        if (!space->support().is_ghost(e))
        {
          // compute norm for these nodes
          boost_foreach( const Uint node, space->connectivity()[e] )
          {
            for (Uint i=0; i<norms.size(); ++i)
              loc_norm[i] += std::pow( std::abs(field[node][i]), (int)order ) ;
          }
        }
      }
    }
  }

  PE::Comm::instance().all_reduce( PE::plus(), &loc_norm[0], norms.size(), &glb_norm[0] );

  for (Uint i=0; i<norms.size(); ++i)
    norms[i] = std::pow(glb_norm[i], 1./order );
}

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < ComputeLNorm, Action, LibSDM > ComputeLNorm_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

ComputeLNorm::ComputeLNorm ( const std::string& name ) : Action(name)
{
  // properties

  properties().add("norm", Real(0.) );

  // options

  options().add("scale", true)
      .description("Scales (divides) the norm by the number of entries (ignored if order zero)");

  options().add("order", 2u)
      .description("Order of the p-norm, zero if L-inf");

  options().add("field", m_field).link_to(&m_field)
      .pretty_name("Field")
      .description("Field to compute norm of");

  options().add("history", m_history).link_to(&m_history);
 }

////////////////////////////////////////////////////////////////////////////////////////////

Uint ComputeLNorm::compute_nb_rows(const Field& field) const
{
  Uint r = 0;
  // loop over all elements
  boost_foreach (const Handle<Space>& space, field.spaces() )
  {
    // only if the elements are volume elements
    if (space->support().element_type().dimension() == space->support().element_type().dimensionality())
    {
      Uint nb_nodes_per_elem = space->shape_function().nb_nodes();

      for (Uint e=0; e<space->size(); ++e)
      {
        if (!space->support().is_ghost(e))
          r += nb_nodes_per_elem;
      }
    }
  }
  return r;
}

////////////////////////////////////////////////////////////////////////////////

std::vector<Real> ComputeLNorm::compute_norm(Field& field) const
{

  const Uint loc_nb_rows = compute_nb_rows(field); // table size on local processor
  Uint nb_rows = 0;                                // table size over all processors

  PE::Comm::instance().all_reduce( PE::plus(), &loc_nb_rows, 1u, &nb_rows );

  if ( !nb_rows ) throw SetupError(FromHere(), "Table is empty");

  std::vector<Real> norm(field.row_size(), 0.);

  const Uint order = options().value<Uint>("order");

  // sum of all processors

  switch(order) {

    case 2:  compute_L2( field, norm );    break;

    case 1:  compute_L1( field, norm );    break;

    case 0:  compute_Linf( field, norm );  break; // consider order 0 as Linf

    default: compute_Lp( field, norm, order );  break;

  }

  if( options().value<bool>("scale") && order )
  {
    for (Uint i=0; i<norm.size(); ++i)
      norm[i] /= nb_rows;
  }

  field.properties()["norm"] = norm;

  return norm;
}

////////////////////////////////////////////////////////////////////////////////

void ComputeLNorm::execute()
{
  if (is_null(m_field)) throw SetupError( FromHere(), "Option 'field' not configured in "+uri().string());
  std::vector<Real> norm = compute_norm(*m_field);
  properties()["norm"] = norm;
  if (m_history)
  {
    for (Uint v=0; v<m_field->nb_vars(); ++v)
    {
      for (Uint j=0; j<m_field->var_length(v); ++j)
      {
        if (m_field->var_length(v) > 1)
        {
          m_history->set("L2("+m_field->descriptor().user_variable_name(v)+"["+to_str(j)+"])",norm[m_field->var_offset(v)+j]);
        }
        else
        {
          m_history->set("L2("+m_field->descriptor().user_variable_name(v)+")",norm[m_field->var_offset(v)+j]);
        }
      }
    }
//    m_history->set("L2("+m_field->name()+")",norm);
  }
}

////////////////////////////////////////////////////////////////////////////////

} // sdm
} // cf3
