// Copyright (C) 2010-2013 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/mpl/vector.hpp>
#include <boost/mpl/for_each.hpp>

#include "common/Builder.hpp"

#include "common/FindComponents.hpp"
#include "common/Foreach.hpp"

#include "common/PE/Comm.hpp"
#include "common/OptionList.hpp"
#include "common/PropertyList.hpp"

#include "mesh/DiscontinuousDictionary.hpp"
#include "mesh/Faces.hpp"
#include "mesh/Region.hpp"
#include "mesh/Space.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/MeshAdaptor.hpp"
#include "mesh/Field.hpp"
#include "mesh/Functions.hpp"
#include "mesh/Connectivity.hpp"
#include "mesh/ElementData.hpp"

#include "MeshDiff.hpp"

//////////////////////////////////////////////////////////////////////////////

namespace cf3 {
namespace mesh {
namespace actions {

////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < MeshDiff, common::Action, mesh::actions::LibActions> MeshDiff_Builder;

////////////////////////////////////////////////////////////////////////////////

namespace detail
{
  
struct DoDiff
{
  DoDiff(const Mesh& left, const Mesh& right, common::Action& differ, bool& mesh_equal) :
    m_left(left),
    m_right(right),
    m_differ(differ),
    m_mesh_equal(mesh_equal)
  {
    m_mesh_equal = true;
    m_prefix = "rank " + common::to_str(common::PE::Comm::instance().rank()) + ": ";
  }
  
  template<typename NumberT>
  void operator()(const NumberT)
  {
    apply< common::List<NumberT> >();
    apply< common::Table<NumberT> >();
  }
  
  template<typename ArrayT>
  void apply()
  {
    BOOST_FOREACH(const ArrayT& left_array, common::find_components_recursively<ArrayT>(m_left))
    {
      std::string relative_path = left_array.uri().path();
      boost::replace_first(relative_path, m_left.uri().path() + "/", "");
      Handle<ArrayT const> right_array(m_right.access_component(common::URI(relative_path, common::URI::Scheme::CPATH)));
      if(is_null(right_array))
      {
        CFdebug << m_prefix << "Array " << relative_path << " was found in " << m_left.uri().path() << " but is missing in " << m_right.uri().path() << CFendl;
        m_mesh_equal = false;
        continue;
      }
      
      m_differ.options().set("left", left_array.handle());
      m_differ.options().set("right", right_array->handle());
      m_differ.execute();
      if(!m_differ.properties().value<bool>("arrays_equal"))
        m_mesh_equal = false;
    }
    
    BOOST_FOREACH(const ArrayT& right_array, common::find_components_recursively<ArrayT>(m_right))
    {
      std::string relative_path = right_array.uri().path();
      boost::replace_first(relative_path, m_right.uri().path() + "/", "");
      Handle<ArrayT const> left_array(m_left.access_component(common::URI(relative_path, common::URI::Scheme::CPATH)));
      if(is_null(left_array))
      {
        CFdebug << m_prefix << "Array " << relative_path << " was missing in " << m_left.uri().path() << " but is present in " << m_right.uri().path() << CFendl;
        m_mesh_equal = false;
      }
    }
  }
  
  const Mesh& m_left;
  const Mesh& m_right;
  common::Action& m_differ;
  bool& m_mesh_equal;
  std::string m_prefix;
};
  
}

MeshDiff::MeshDiff(const std::string& name) : common::Action(name)
{
  options().add("left", Handle<Mesh>())
    .pretty_name("Left")
    .description("Left mesh in the comparison")
    .mark_basic();
    
  options().add("right", Handle<Mesh>())
    .pretty_name("Right")
    .description("Right mesh in the comparison")
    .mark_basic();
    
  options().add("max_ulps", 10u)
    .pretty_name("Max ULPs")
    .description("Maximum distance allowed between floating point numbers")
    .mark_basic();
    
  options().add("zero_threshold", 1e-16)
    .pretty_name("Zero Threshold")
    .description("Floating point numbers smaller than this are considered to be zero")
    .mark_basic();

  properties().add("mesh_equal", false);
}

void MeshDiff::execute()
{
  Handle<Mesh> left = options().value< Handle<Mesh> >("left");
  Handle<Mesh> right = options().value< Handle<Mesh> >("right");
  
  if(is_null(left))
    throw common::SetupError(FromHere(), "Left mesh is not set in " + uri().path());
  
  if(is_null(right))
    throw common::SetupError(FromHere(), "Right mesh is not set in " + uri().path());
  
  // Setup a differ for the arrays
  Handle<common::Action> array_differ(get_child("ArrayDiff"));
  if(is_null(array_differ))
    array_differ = Handle<common::Action>(create_component("ArrayDiff", "cf3.common.ArrayDiff"));
  array_differ->options().set("max_ulps", options().value<Uint>("max_ulps"));
  array_differ->options().set("zero_threshold", options().value<Real>("zero_threshold"));
  
  // Compare all arrays and lists in the mesh
  typedef boost::mpl::vector4<int, Uint, float, double> allowed_types;
  bool mesh_equal = true;
  boost::mpl::for_each<allowed_types>(detail::DoDiff(*left, *right, *array_differ, mesh_equal));
  properties().set("mesh_equal", mesh_equal);
}

//////////////////////////////////////////////////////////////////////////////


} // actions
} // mesh
} // cf3
