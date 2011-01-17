// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/regex.hpp>

#include "Common/Log.hpp"
#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/String/Conversion.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CFieldRegion.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CFieldElements.hpp"

namespace CF {
namespace Mesh {

using namespace boost::assign;
using namespace Common;
using namespace Common::String;

Common::ComponentBuilder < CFieldRegion, Component, LibMesh >  CFieldRegion_Builder;

////////////////////////////////////////////////////////////////////////////////

CFieldRegion::CFieldRegion ( const std::string& name  ) :
  CRegion ( name )
{
  m_support = create_static_component<CLink>("support");
  m_support->add_tag("support"); 
}

////////////////////////////////////////////////////////////////////////////////

CFieldRegion::~CFieldRegion()
{
}

////////////////////////////////////////////////////////////////////////////////

CFieldRegion& CFieldRegion::synchronize_with_region(CRegion& support, const std::string& tree_name)
{
  m_support->link_to(support.self());
  // Setup this field
  m_tree_name = (tree_name == "") ? name() : tree_name;
  //support.add_field_link(*this);

  // Create FieldElements if the support has them
  boost_foreach(CElements& geometry_elements, find_components<CElements>(support))
    create_elements(geometry_elements);

  // Go down one level in the tree
  boost_foreach(CRegion& support_level_down, find_components<CRegion>(support))
  {
    CFieldRegion& subfield = *create_component<CFieldRegion>(support_level_down.name());
    subfield.synchronize_with_region(support_level_down,m_tree_name);
  }
  return *this;
}

////////////////////////////////////////////////////////////////////////////////

CElements& CFieldRegion::create_elements(CElements& geometry_elements)
{
  CElements& field_elements = *create_component<CElements>(geometry_elements.name());
  field_elements.initialize(geometry_elements);
  return field_elements;
}

//////////////////////////////////////////////////////////////////////////////

const CRegion& CFieldRegion::support() const
{
  return *m_support->follow()->as_type<CRegion>();  // follow() because it is a link
}

//////////////////////////////////////////////////////////////////////////////

CRegion& CFieldRegion::support()
{
  return *m_support->follow()->as_type<CRegion>();  // follow() because it is a link
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
