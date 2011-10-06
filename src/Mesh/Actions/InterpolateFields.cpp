// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/Actions/InterpolateFields.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CSpace.hpp"
#include "Mesh/Field.hpp"
#include "Mesh/ShapeFunction.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Actions {

using namespace Common;
using namespace Common::PE;

////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < InterpolateFields, CMeshTransformer, LibActions> InterpolateFields_Builder;

//////////////////////////////////////////////////////////////////////////////

InterpolateFields::InterpolateFields( const std::string& name )
: CMeshTransformer(name)
{

  properties()["brief"] = std::string("Interpolate Fields with matching support");
  std::string desc;
  desc =
    "Shapefunction/Space of the source-field is used to interpolate to the target";
  properties()["description"] = desc;

  m_options.add_option(OptionComponent<Field>::create("source", &m_source))
      ->description("Field to interpolate from")
      ->pretty_name("Source Field")
      ->mark_basic();

  m_options.add_option(OptionComponent<Field>::create("target", &m_target))
      ->description("Field to interpolate to")
      ->pretty_name("TargetField")
      ->mark_basic();
}

/////////////////////////////////////////////////////////////////////////////

void InterpolateFields::execute()
{
  if (m_source.expired())
    throw SetupError(FromHere(),uri().string()+": Source field not configured");

  if (m_target.expired())
    throw SetupError(FromHere(),uri().string()+": Target field not configured");

  const Field& source = *m_source.lock();
  Field& target = *m_target.lock();

  if (source.row_size() != target.row_size())
    throw BadValue(FromHere(),"Field "+source.uri().string()+" has "+to_str(source.row_size())+" variables.\n"
                              "Field "+target.uri().string()+" has "+to_str(target.row_size())+" variables.");
  const Uint nb_vars(source.row_size());

  /// Loop over Regions
  boost_foreach(CEntities& elements, target.entities_range())
  {
    if (source.elements_lookup().contains(elements) == false)
      continue;
//      throw BadValue(FromHere(),"Source field "+source.uri().string()+" is not defined in elements "+elements.uri().string());

    const CSpace& s_space = source.space(elements);
    const CSpace& t_space = target.space(elements);
    const ShapeFunction& s_sf = s_space.shape_function();
    const ShapeFunction& t_sf = t_space.shape_function();

    /// Compute Interpolation matrix, equal for every element
    RealMatrix interpolate(t_sf.nb_nodes(),s_sf.nb_nodes());
    for (Uint t_pt = 0; t_pt<t_sf.nb_nodes(); ++t_pt)
      interpolate.row(t_pt) = s_sf.value(t_sf.local_coordinates().row(t_pt));

    /// Element loop
    for (Uint e=0; e<elements.size(); ++e)
    {
      CConnectivity::ConstRow s_field_indexes = s_space.indexes_for_element(e);
      CConnectivity::ConstRow t_field_indexes = t_space.indexes_for_element(e);

      /// Interpolate: target[element] = interpolate * source[element]
      /// Split in loops since we cannot work with Matrix-products
      for (Uint t_pt=0; t_pt<t_sf.nb_nodes(); ++t_pt)
      {
        for (Uint var=0; var<nb_vars; ++var)
          target[t_field_indexes[t_pt]][var] = 0.;

        for (Uint s_pt=0; s_pt<s_sf.nb_nodes(); ++s_pt)
        {
          for (Uint var=0; var<nb_vars; ++var)
            target[t_field_indexes[t_pt]][var] += interpolate(t_pt,s_pt) * source[s_field_indexes[s_pt]][var];
        }
      }
    }
  }
}

//////////////////////////////////////////////////////////////////////////////


} // Actions
} // Mesh
} // CF
