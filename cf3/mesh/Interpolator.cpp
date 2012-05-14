// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/function.hpp>
#include <boost/bind.hpp>

#include "math/MatrixTypesConversion.hpp"

#include "common/FindComponents.hpp"
#include "common/Builder.hpp"
#include "common/OptionComponent.hpp"
#include "common/OptionList.hpp"
#include "common/OptionT.hpp"

#include "mesh/Interpolator.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"

#include "mesh/PointInterpolator.hpp"

#include "common/OptionList.hpp"

namespace cf3 {
namespace mesh {

using namespace common;
using namespace common::XML;

////////////////////////////////////////////////////////////////////////////////

Interpolator::Interpolator(const std::string &name) : Component(name)
{
  m_point_interpolator = create_component<PointInterpolator>("point_interpolator");

  options().add_option("store", true)
      .description("Flag to store weights and stencils used for faster interpolation")
      .pretty_name("Store");
}

////////////////////////////////////////////////////////////////////////////////

void Interpolator::interpolate(const Field& source_field, Field& target_field)
{
  if (options().option("store").value<bool>())
  {
    if (m_stored_element.size() == 0 || (target_field.size() != m_stored_element.size())) // if nothing is stored yet, store it
    {

      cf3_assert(m_point_interpolator);
      m_point_interpolator->options().configure_option("source",source_field.dict().handle<Dictionary>());

      m_stored_element.resize(target_field.size());
      m_stored_stencil.resize(target_field.size());
      m_stored_source_field_points.resize(target_field.size());
      m_stored_source_field_weights.resize(target_field.size());

      const Field& target_coordinates = target_field.coordinates();
      RealVector t_point(target_coordinates.row_size());
      for (Uint t=0; t<target_field.size(); ++t)
      {
        math::copy(target_coordinates[t],t_point);
        bool interpolation_possible =
            m_point_interpolator->compute_storage(t_point,
                                                  m_stored_element[t],
                                                  m_stored_stencil[t],
                                                  m_stored_source_field_points[t],
                                                  m_stored_source_field_weights[t]);
        if (!interpolation_possible)
          throw ValueNotFound(FromHere(),"coordinate not found");
      }
    }

    if (target_field.size() != m_stored_element.size())
      throw InvalidStructure(FromHere(),"The stored values for speedy interpolation are invalid for target field "+target_field.uri().string());

    target_field = 0.;
    for (Uint t=0; t<target_field.size(); ++t)
    {
      for (Uint v=0; v<target_field.row_size(); ++v)
      {
        for (Uint s=0; s<m_stored_source_field_points[t].size(); ++s)
          target_field[t][v] += source_field[ m_stored_source_field_points[t][s] ][v] * m_stored_source_field_weights[t][s];
      }
    }
  }
  else
  {
    cf3_assert(m_point_interpolator);
    m_point_interpolator->options().configure_option("source",source_field.dict().handle<Dictionary>());

    const Field& target_coordinates = target_field.coordinates();
    RealVector t_point(target_coordinates.row_size());
    for (Uint t=0; t<target_field.size(); ++t)
    {
      math::copy(target_coordinates[t],t_point);
      Field::ArrayT::reference ref = target_field[t];
      m_point_interpolator->interpolate(source_field,t_point,ref);
    }
  }
}

////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////
////////////////////////////////////////////////////////////////////////////////


OldInterpolator::OldInterpolator ( const std::string& name  ) :
  Component ( name )
{
  options().add_option("source", m_source)
      .description("Field to interpolate from")
      .pretty_name("Source Field")
      .mark_basic()
      .link_to(&m_source);

  options().add_option("target", m_target)
      .description("Field to interpolate to")
      .pretty_name("TargetField")
      .mark_basic()
      .link_to(&m_target);

  options().add_option("store", true)
      .description("Flag to store weights and stencils used for faster interpolation")
      .pretty_name("Store");
}

////////////////////////////////////////////////////////////////////////////////

OldInterpolator::~OldInterpolator()
{
}

//////////////////////////////////////////////////////////////////////////////

void OldInterpolator::signal_interpolate( SignalArgs& node  )
{
  interpolate();
}

////////////////////////////////////////////////////////////////////////////////

void OldInterpolator::interpolate()
{
  if ( is_null(m_source) )
    throw SetupError (FromHere(), "SourceField option was not set");
  if ( is_null(m_target) )
    throw SetupError (FromHere(), "TargetField option was not set");
  construct_internal_storage(*Handle<Mesh>(m_source->parent()));
  interpolate_field_from_to(*m_source,*m_target);
}

////////////////////////////////////////////////////////////////////////////////

} // mesh
} // cf3
