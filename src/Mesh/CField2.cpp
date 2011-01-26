// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/regex.hpp>

#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/String/Conversion.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CField2.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

using namespace boost::assign;
using namespace Common;
using namespace Common::String;

Common::ComponentBuilder < CField2, Component, LibMesh >  CField2_Builder;

////////////////////////////////////////////////////////////////////////////////

CField2::CField2 ( const std::string& name  ) :
  Component ( name ),
  m_registration_name ( name )
{
  Option::Ptr uri_option;
  uri_option = m_properties.add_option<OptionURI>("Topology","The field tree this field will be registered in",URI("cpath:"));
  uri_option->attach_trigger ( boost::bind ( &CField2::config_tree,   this ) );
  uri_option->mark_basic();
  
  Option::Ptr option;
  option = m_properties.add_option< OptionT<std::string> >("FieldType", "The type of the field", std::string("NodeBased"));
  option->restricted_list() += std::string("ElementBased");  
  option->attach_trigger ( boost::bind ( &CField2::config_field_type,   this ) );
  option->mark_basic();
  
  std::vector<std::string> var_names;
  var_names.push_back(name);
  option = m_properties.add_option<OptionArrayT<std::string> >("VarNames","Names of the variables",var_names);
  option->attach_trigger ( boost::bind ( &CField2::config_var_names,   this ) );
  option->mark_basic();
  config_var_names();
  
  std::vector<std::string> var_types;
  var_types.push_back("scalar");
  option = m_properties.add_option<OptionArrayT<std::string> >("VarTypes","Types of the variables",var_types);
  option->restricted_list() += std::string("scalar") ,
                               std::string("vector2D"),
                               std::string("vector3D"),
                               std::string("tensor2D"),
                               std::string("tensor3D");
  option->attach_trigger ( boost::bind ( &CField2::config_var_types,   this ) );
  option->mark_basic();
  config_var_types();

  
  std::vector<Uint> var_sizes;
  var_sizes.push_back(1);
  option = m_properties.add_option<OptionArrayT<Uint> >("VarSizes","Sizes of the variables",var_sizes);
  option->attach_trigger ( boost::bind ( &CField2::config_var_sizes,   this ) );
  config_var_sizes();

  m_topology = create_static_component<CLink>("topology");
  m_data = create_static_component<CTable<Real> >("data");
  
}
  
////////////////////////////////////////////////////////////////////////////////

void CField2::config_var_types()
{
  std::vector<std::string> var_types; property("VarTypes").put_value(var_types);
  
  boost::regex e_scalar  ("(s(cal(ar)?)?)|1"     ,boost::regex::perl|boost::regex::icase);
  boost::regex e_vector2d("(v(ec(tor)?)?.?2d?)|2",boost::regex::perl|boost::regex::icase);
  boost::regex e_vector3d("(v(ec(tor)?)?.?3d?)|3",boost::regex::perl|boost::regex::icase);
  boost::regex e_tensor2d("(t(ens(or)?)?.?2d?)|4",boost::regex::perl|boost::regex::icase);
  boost::regex e_tensor3d("(t(ens(or)?)?.?3d?)|9",boost::regex::perl|boost::regex::icase);
  
  m_var_types.resize(var_types.size());
  Uint iVar = 0;
  boost_foreach (const std::string& var_type, var_types)
  {
    if (boost::regex_match(var_type,e_scalar))
      m_var_types[iVar++]=SCALAR;
    else if (boost::regex_match(var_type,e_vector2d))
      m_var_types[iVar++]=VECTOR_2D;
    else if (boost::regex_match(var_type,e_vector3d))
      m_var_types[iVar++]=VECTOR_3D;
    else if (boost::regex_match(var_type,e_tensor2d))
      m_var_types[iVar++]=TENSOR_2D;
    else if (boost::regex_match(var_type,e_tensor3d))
      m_var_types[iVar++]=TENSOR_3D;
  }
  
  std::vector<Uint> var_sizes(m_var_types.size());
  for (Uint i=0; i<m_var_types.size(); ++i)
    var_sizes[i]=Uint(m_var_types[i]);
  boost_foreach(CField2& subfield, find_components<CField2>(*this))
    subfield.configure_property("VarSizes",var_sizes);
  
}

////////////////////////////////////////////////////////////////////////////////

void CField2::config_var_names()
{
  property("VarNames").put_value(m_var_names);
}

////////////////////////////////////////////////////////////////////////////////

void CField2::config_var_sizes()
{
  std::vector<Uint> var_sizes; property("VarSizes").put_value(var_sizes);
  Uint iVar=0;
  m_var_types.resize(var_sizes.size());
  boost_foreach(Uint var_type, var_sizes)
  m_var_types[iVar++]=VarType(var_type);
  
  boost_foreach(CField2& subfield, find_components<CField2>(*this))
    subfield.configure_property("VarSizes",var_sizes);  
}

////////////////////////////////////////////////////////////////////////////////

void CField2::config_field_type()
{
  std::string field_type;
  property("FieldType").put_value(field_type);
  if (field_type == "NodeBased")
    m_basis = NODE_BASED;
  else
    m_basis = ELEMENT_BASED;
}

////////////////////////////////////////////////////////////////////////////////

std::string CField2::var_name(Uint i) const
{
  cf_assert(i<m_var_types.size());
  return m_var_names.size() ? m_var_names[i] : "var";
  
  //  std::vector<std::string> names;
  //  switch (m_var_types[i])
  //  {
  //    case SCALAR:
  //      names += name;
  //      break;
  //    case VECTOR_2D:
  //      names += name+"x";
  //      names += name+"y";
  //      break;
  //    case VECTOR_3D:
  //      names += name+"x";
  //      names += name+"y";
  //      names += name+"z";
  //      break;
  //    case TENSOR_2D:
  //      names += name+"xx";
  //      names += name+"xy";
  //      names += name+"yx";
  //      names += name+"yy";
  //      break;
  //    case TENSOR_3D:
  //      names += name+"xx";
  //      names += name+"xy";
  //      names += name+"xz";
  //      names += name+"yx";
  //      names += name+"yy";
  //      names += name+"yz";
  //      names += name+"zx";
  //      names += name+"zy";
  //      names += name+"zz";
  //      break;
  //    default:
  //      break;
  //  }
  //  return names;
}
  
////////////////////////////////////////////////////////////////////////////////

void CField2::config_tree()
{
  URI topology_uri;
  property("Topology").put_value(topology_uri);
  CRegion::Ptr topology = Core::instance().root()->look_component<CRegion>(topology_uri);
  if ( is_null(topology) )
    throw CastingFailed (FromHere(), "Topology must be of a CRegion or derived type");
  m_topology->link_to(topology);
}

////////////////////////////////////////////////////////////////////////////////

CField2::~CField2()
{
}

////////////////////////////////////////////////////////////////////////////////


void CField2::create_data_storage()
{

  cf_assert( m_var_types.size()!=0 );
  cf_assert( is_not_null(m_topology->follow()) );
  
  
  // Check if there are coordinates in this field, and add to map
  m_coords = find_parent_component<CMesh>(topology()).nodes().coordinates().as_type<CTable<Real> >();

  Uint row_size(0);
  boost_foreach(const VarType var_size, m_var_types)
    row_size += Uint(var_size);
  
  m_data->set_row_size(row_size);
  
  switch (m_basis)
  {
    case ELEMENT_BASED:
    {
      Uint data_size = 0;
      boost_foreach(CElements& field_elements, find_components_recursively<CElements>(topology()))
      {
        m_elements_start_idx[&field_elements] = data_size;
        CFieldView field_view("tmp_field_view");
        data_size = field_view.initialize(*this);
      }
      m_data->resize(data_size);
      break;      
    }
    case NODE_BASED:
    {
      m_used_nodes = CElements::used_nodes(topology()).as_type<CList<Uint> >();
      m_data->resize(m_used_nodes->size());
    }
      break;
    default:
      throw NotSupported(FromHere() , "DataBasis can only be ELEMENT_BASED or NODE_BASED");
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

Uint CField2::var_number ( const std::string& vname ) const
{
  const std::vector<std::string>::const_iterator var_loc_it = std::find(m_var_names.begin(), m_var_names.end(), vname);
  if(var_loc_it == m_var_names.end())
    throw Common::ValueNotFound(FromHere(), "Variable " + vname + " was not found in field " + name());
  return var_loc_it - m_var_names.begin();
}
//////////////////////////////////////////////////////////////////////////////

Uint CField2::var_index ( const std::string& vname ) const
{
  const Uint var_nb = var_number(vname);
  Uint var_start = 0;
  for(Uint i = 0; i != var_nb; ++i)
    var_start += m_var_types[i];
  return var_start;
}

//////////////////////////////////////////////////////////////////////////////

CField2::VarType CField2::var_type ( const std::string& vname ) const
{
  return var_type(var_number(vname));
}

////////////////////////////////////////////////////////////////////////////////

const CRegion& CField2::topology() const
{
  return *m_topology->follow()->as_type<CRegion>();
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CField2::topology()
{
  return *m_topology->follow()->as_type<CRegion>();
}

////////////////////////////////////////////////////////////////////////////////

const CList<Uint>& CField2::used_nodes() const
{
  return *m_used_nodes;
}

//////////////////////////////////////////////////////////////////////////////  

CTable<Real>::ConstRow CField2::coords(const Uint idx) const
{
  return (*m_coords)[used_nodes()[idx]];
}

////////////////////////////////////////////////////////////////////////////////

Uint CFieldView::initialize(CField2& field, CElements::Ptr elements)
{

  // If run-time error occurs in this if-statement, the default argument must be given a valid CElements::Ptr
  if (is_null(elements))
    elements = find_parent_component<CElements>(*this).as_type<CElements>();

  set_field(field);  
  set_elements(elements);
  return m_end_idx;
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
