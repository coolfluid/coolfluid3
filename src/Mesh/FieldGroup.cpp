// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>

#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/CBuilder.hpp"
#include "Common/CLink.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/FieldGroup.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CList.hpp"


namespace CF {
namespace Mesh {

using namespace Common;
using namespace boost::assign;

Common::ComponentBuilder < FieldGroup, Component, LibMesh >  FieldGroup_Builder;

////////////////////////////////////////////////////////////////////////////////

FieldGroup::Basis::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
      ( FieldGroup::Basis::POINT_BASED, "point_based" )
      ( FieldGroup::Basis::ELEMENT_BASED, "element_based" )
      ( FieldGroup::Basis::CELL_BASED, "cell_based" )
      ( FieldGroup::Basis::FACE_BASED, "face_based" );

  all_rev = boost::assign::map_list_of
      ("point_based",    FieldGroup::Basis::POINT_BASED )
      ("element_based",  FieldGroup::Basis::ELEMENT_BASED )
      ("cell_based",     FieldGroup::Basis::CELL_BASED )
      ("face_based",     FieldGroup::Basis::FACE_BASED );
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup::Basis::Convert& FieldGroup::Basis::Convert::instance()
{
  static FieldGroup::Basis::Convert instance;
  return instance;
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup::FieldGroup ( const std::string& name  ) :
  CNodes( name ),
  m_basis(Basis::POINT_BASED),
  m_space("space[0]")
{
  mark_basic();

  // Option "topology"
  m_options.add_option< OptionURI >("topology",URI("cpath:"))
      ->set_description("The region these fields apply to")
      ->attach_trigger( boost::bind( &FieldGroup::config_topology, this) )
      ->mark_basic();

  // Option "type"
  m_options.add_option< OptionT<std::string> >("type", Basis::to_str(m_basis))
      ->set_description("The type of the field")
      ->attach_trigger ( boost::bind ( &FieldGroup::config_type,   this ) )
      ->mark_basic();
  option("type").restricted_list() =  list_of
      (Basis::to_str(Basis::POINT_BASED))
      (Basis::to_str(Basis::ELEMENT_BASED))
      (Basis::to_str(Basis::CELL_BASED))
      (Basis::to_str(Basis::FACE_BASED));

  // Option "space
  m_options.add_option< OptionT<std::string> >("space", m_space)
    ->set_description("The space of the field is based on")
    ->link_to(&m_space)
    ->mark_basic();

  // Static components
  m_topology = create_static_component_ptr<CLink>("topology");

//  m_glb_idx = create_static_component_ptr<CList<Uint> >("glb_idx");

//  m_rank = create_static_component_ptr<CList<Uint> >("rank");

}

////////////////////////////////////////////////////////////////////////////////

void FieldGroup::config_topology()
{
  URI topology_uri;
  option("topology").put_value(topology_uri);
  CRegion::Ptr topology = Core::instance().root().access_component(topology_uri).as_ptr<CRegion>();
  if ( is_null(topology) )
    throw CastingFailed (FromHere(), "Topology must be of a CRegion or derived type");
  m_topology->link_to(topology);
}

////////////////////////////////////////////////////////////////////////////////

void FieldGroup::config_type()
{
  m_basis = Basis::to_enum( option("type").value<std::string>() );
}

////////////////////////////////////////////////////////////////////////////////

FieldGroup::~FieldGroup()
{
}


////////////////////////////////////////////////////////////////////////////////

//void FieldGroup::resize(const Uint size)
//{
//  m_size = size;
//  m_glb_idx->resize(m_size);
//  m_rank->resize(m_size);
//  properties()["size"]=m_size;
//}

////////////////////////////////////////////////////////////////////////////////

const CRegion& FieldGroup::topology() const
{
  return *m_topology->follow()->as_ptr<CRegion>();
}

////////////////////////////////////////////////////////////////////////////////

CRegion& FieldGroup::topology()
{
  return *m_topology->follow()->as_ptr<CRegion>();
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
