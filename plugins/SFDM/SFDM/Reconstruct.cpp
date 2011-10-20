// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Log.hpp"
#include "common/CBuilder.hpp"
#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"
#include "common/CBuilder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionArray.hpp"
#include "common/StringConversion.hpp"
#include "Mesh/GeoShape.hpp"

#include "SFDM/Reconstruct.hpp"
#include "SFDM/ShapeFunction.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace SFDM {

  using namespace common;

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Reconstruct, Component, LibSFDM> Reconstruct_Builder;

//////////////////////////////////////////////////////////////////////////////

Reconstruct::Reconstruct( const std::string& name )
: Component(name)
{

  m_properties["brief"] = std::string("Reconstruction between 2 shape functions");
  m_properties["description"] = std::string("Perform reconstruction between 2 shapefunctions inside one element");

  std::vector<std::string> from_to(2);
  from_to[0] = "e.g. CF.SFDM.SF.LineSolutionP2";
  from_to[1] = "e.g. CF.SFDM.SF.LineFluxP3";
  m_options.add_option( OptionArrayT<std::string>::create("from_to", from_to ) )
      ->description("Shape function points from where the states are known")
      ->pretty_name("From and To Shape Function")
      ->mark_basic()
      ->attach_trigger( boost::bind ( &Reconstruct::configure_from_to , this ) );

}

/////////////////////////////////////////////////////////////////////////////

void Reconstruct::configure_from_to()
{
  std::vector<std::string> from_to; option("from_to").put_value(from_to);

  if (is_not_null(m_from))
    remove_component(*m_from);
  m_from = create_component("from_"+from_to[0],from_to[0]).as_ptr<ShapeFunction>();
  if (is_not_null(m_to))
    remove_component(*m_to);
  m_to   = create_component("to_"+from_to[1],from_to[1]).as_ptr<ShapeFunction>();

  m_value_reconstruction_matrix.resize(m_to->nb_nodes(),m_from->nb_nodes());
  m_gradient_reconstruction_matrix.resize(m_to->dimensionality());
  for (Uint d=0; d<m_gradient_reconstruction_matrix.size(); ++d)
    m_gradient_reconstruction_matrix[d].resize(m_to->nb_nodes(),m_from->nb_nodes());
  for (Uint to_node=0; to_node<m_to->nb_nodes(); ++to_node)
  {
    m_value_reconstruction_matrix.row(to_node) = m_from->value( m_to->local_coordinates().row(to_node) );
    RealMatrix grad = m_from->gradient( m_to->local_coordinates().row(to_node) );
    for (Uint d=0; d<m_to->dimensionality(); ++d)
      m_gradient_reconstruction_matrix[d].row(to_node) = grad.row(d);
  }
}

/////////////////////////////////////////////////////////////////////////////

RealMatrix Reconstruct::value(const RealMatrix& from_states) const
{
  cf_assert_desc("matrix dimensions don't match ["+to_str((Uint)from_states.rows())+"!="+to_str((Uint)m_value_reconstruction_matrix.cols())+"]  ",from_states.rows() == m_value_reconstruction_matrix.cols());
  return m_value_reconstruction_matrix * from_states;
}

/////////////////////////////////////////////////////////////////////////////

RealMatrix Reconstruct::gradient(const RealMatrix& from_states, const CoordRef orientation) const
{
  cf_assert_desc("matrix dimensions don't match ["+to_str((Uint)from_states.rows())+"!="+to_str((Uint)m_gradient_reconstruction_matrix[orientation].cols())+"]  ",from_states.rows() == m_gradient_reconstruction_matrix[orientation].cols());
  return m_gradient_reconstruction_matrix[orientation] * from_states;
}

//////////////////////////////////////////////////////////////////////////////

} // SFDM
} // CF
