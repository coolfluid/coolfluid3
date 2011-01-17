// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/regex.hpp>

#include "Common/CBuilder.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"
#include "Common/String/Conversion.hpp"
#include "Common/Log.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CFieldElements.hpp"
#include "Mesh/CMesh.hpp"

namespace CF {
namespace Mesh {

using namespace boost::assign;
using namespace Common;
using namespace Common::String;

Common::ComponentBuilder < CField, Component, LibMesh >  CField_Builder;

////////////////////////////////////////////////////////////////////////////////

CField::CField ( const std::string& name  ) :
  Component ( name )
{


  std::vector<std::string> var_names;
  std::vector<std::string> var_types;
  std::vector<Uint> var_sizes;
  m_properties.add_option<OptionArrayT<std::string> >("VarNames","Names of the variables",var_names)->mark_basic();
  m_properties.add_option<OptionArrayT<std::string> >("VarTypes","Types of the variables",var_names)->mark_basic();
  m_properties.add_option<OptionArrayT<Uint>        >("VarSizes","Sizes of the variables",var_sizes)->mark_basic();

  m_properties["VarNames"].as_option().attach_trigger ( boost::bind ( &CField::config_var_names,   this ) );
  m_properties["VarSizes"].as_option().attach_trigger ( boost::bind ( &CField::config_var_sizes,   this ) );
  m_properties["VarTypes"].as_option().attach_trigger ( boost::bind ( &CField::config_var_types,   this ) );

  m_support = create_static_component<CLink>("support");
  m_support->add_tag("support");
  
}
  
////////////////////////////////////////////////////////////////////////////////

void CField::config_var_types()
{
  std::vector<std::string> var_types; property("VarTypes").put_value(var_types);
  
  boost::regex e_scalar  ("(s(cal(ar)?)?)|1"     ,boost::regex::perl|boost::regex::icase);
  boost::regex e_vector2d("(v(ec(tor)?)?.?2d?)|2",boost::regex::perl|boost::regex::icase);
  boost::regex e_vector3d("(v(ec(tor)?)?.?3d?)|3",boost::regex::perl|boost::regex::icase);
  boost::regex e_tensor2d("(t(ens(or)?)?.?2d?)|4",boost::regex::perl|boost::regex::icase);
  boost::regex e_tensor3d("(t(ens(or)?)?.?3d?)|9",boost::regex::perl|boost::regex::icase);
  
  m_var_types.resize(var_types.size());
  Uint iVar = 0;
  BOOST_FOREACH (const std::string& var_type, var_types)
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
  BOOST_FOREACH(CField& subfield, find_components<CField>(*this))
    subfield.configure_property("VarSizes",var_sizes);
  
}

////////////////////////////////////////////////////////////////////////////////

void CField::config_var_names()
{
  property("VarNames").put_value(m_var_names);
  
  BOOST_FOREACH(CField& subfield, find_components<CField>(*this))
    subfield.configure_property("VarNames",m_var_names);
}

////////////////////////////////////////////////////////////////////////////////

void CField::config_var_sizes()
{
  std::vector<Uint> var_sizes; property("VarSizes").put_value(var_sizes);
  Uint iVar=0;
  m_var_types.resize(var_sizes.size());
  BOOST_FOREACH(Uint var_type, var_sizes)
  m_var_types[iVar++]=VarType(var_type);
  
  BOOST_FOREACH(CField& subfield, find_components<CField>(*this))
    subfield.configure_property("VarSizes",var_sizes);  
}

////////////////////////////////////////////////////////////////////////////////

std::string CField::var_name(Uint i) const
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

CField::~CField()
{
}

////////////////////////////////////////////////////////////////////////////////

CField& CField::synchronize_with_region(CRegion& support, const std::string& field_name)
{
  // Setup this field
  m_field_name = (field_name == "") ? name() : field_name;
  support.add_field_link(*this);
  m_support->link_to(support.self());

  // Create FieldElements if the support has them
  boost_foreach(CElements& geometry_elements, find_components<CElements>(support))
    create_elements(geometry_elements);

  // Go down one level in the tree
  boost_foreach(CRegion& support_level_down, find_components<CRegion>(support))
  {
    CField& subfield = *create_component<CField>(support_level_down.name());
    subfield.synchronize_with_region(support_level_down,m_field_name);
  }

  return *this;
}
  
////////////////////////////////////////////////////////////////////////////////


void CField::create_data_storage(const DataBasis basis)
{
  m_basis = basis;
  BOOST_FOREACH(CField& subfield, find_components_recursively<CField>(*this))
  {
    subfield.set_basis(m_basis);
  }

  CNodes& nodes = find_parent_component<CMesh>(*this).nodes();


  cf_assert(m_var_types.size()!=0);
  
  Uint row_size(0);
  BOOST_FOREACH(const VarType var_size, m_var_types)
    row_size += Uint(var_size);
  
  switch (m_basis)
  {
    case ELEMENT_BASED:
      BOOST_FOREACH(CFieldElements& field_elements, find_components_recursively<CFieldElements>(*this))
      {
        field_elements.add_element_based_storage();
        field_elements.data().array().resize(boost::extents[field_elements.elements_count()][row_size]);
      }
      break;
    case NODE_BASED:
    {
      std::map<std::string,CTable<Real>*> data_for_nodes;

      // Check if there are coordinates in this field, and add to map
      CTable<Real>::Ptr field_data = create_component<CTable<Real> >("data");
      field_data->add_tag("field_data");
      field_data->set_row_size(row_size);
      field_data->resize(nodes.size());
      data_for_nodes[nodes.full_path().string_without_scheme()] = field_data.get();
      
      // create a link to the coordinates in the data
      CLink::Ptr nodes_link = field_data->create_component<CLink>("nodes");
      nodes_link->link_to(nodes.follow());
      nodes_link->add_tag("nodes_link");

			boost_foreach(CField& subfield, find_components_recursively<CField>(*this))
			{
          CLink::Ptr data_link = subfield.create_component<CLink>("data");
          data_link->add_tag("field_data");
          data_link->link_to(field_data);
			}

      // Add the correct data according to the map in every field elements component
      boost_foreach(CFieldElements& field_elements, find_components_recursively<CFieldElements>(*this))
      {
        field_elements.add_node_based_storage(*field_data);
      }
    }
      break;
    default:
      throw NotSupported(FromHere() , "DataBasis can only be ELEMENT_BASED or NODE_BASED");
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

//CField& CField::create_element_based_field(const std::string& field_name, CRegion& support)
//{
//  m_field_name = field_name;
//  support.add_field_link(*this);
//  create_component<CLink>("support")->link_to(support.get());
//
//  BOOST_FOREACH(CElements& geometry_elements, find_components<CElements>(support))
//  {
//    CFinfo << "creating elements element_based" << geometry_elements.name() << CFendl;
//    CFieldElements& field_elements = *create_component<CFieldElements>(geometry_elements.name());
//    field_elements.add_tag("FieldElements");
//    field_elements.initialize(geometry_elements);
//    field_elements.add_element_based_storage();
//    geometry_elements.add_field_elements_link(field_elements);
//  }
//
//  BOOST_FOREACH(CRegion& support_level_down, find_components<CRegion>(support))
//  {
//    CField& field = *create_component<CField>(support_level_down.name());
//    field.create_element_based_field(m_field_name,support_level_down);
//  }
//  return *this;
//}

////////////////////////////////////////////////////////////////////////////////

CElements& CField::create_elements(CElements& geometry_elements)
{
  CFieldElements& field_elements = *create_component<CFieldElements>(geometry_elements.name());
  field_elements.initialize(geometry_elements);
  geometry_elements.add_field_elements_link(field_elements);
  return field_elements;
}

//////////////////////////////////////////////////////////////////////////////

const CRegion& CField::support() const
{
  return *m_support->follow()->as_type<CRegion>();  // get() because it is a link
}

//////////////////////////////////////////////////////////////////////////////

CRegion& CField::support()
{
  return *m_support->follow()->as_type<CRegion>();  // get() because it is a link
}

////////////////////////////////////////////////////////////////////////////////

const CField& CField::subfield(const std::string& name) const
{
  return find_component_with_name<CField const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CField& CField::subfield(const std::string& name)
{
  return find_component_with_name<CField>(*this,name);
}

//////////////////////////////////////////////////////////////////////////////

const CFieldElements& CField::elements(const std::string& name) const
{
  return find_component_with_name<CFieldElements const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CFieldElements& CField::elements(const std::string& name)
{
  return find_component_with_name<CFieldElements>(*this,name);
}

Uint CField::find_var ( const std::string& vname ) const
{
  const std::vector<std::string>::const_iterator var_loc_it = std::find(m_var_names.begin(), m_var_names.end(), vname);
  if(var_loc_it == m_var_names.end())
    throw Common::ValueNotFound(FromHere(), "Variable " + vname + " was not found in field " + field_name());
  return var_loc_it - m_var_names.begin();
}
//////////////////////////////////////////////////////////////////////////////

Uint CField::var_index ( const std::string& vname ) const
{
  const Uint var_number = find_var(vname);
  Uint var_start = 0;
  for(Uint i = 0; i != var_number; ++i)
    var_start += m_var_types[i];
  return var_start;
}

//////////////////////////////////////////////////////////////////////////////

Uint CField::var_length ( const std::string& vname ) const
{
  return var_type(find_var(vname));
}

//////////////////////////////////////////////////////////////////////////////  

CTable<Real>& CField::data_table()
{
  Component::Ptr data = find_component_ptr_with_tag(*this, "field_data");
  if(!data)
    throw ValueNotFound(FromHere(), "Field " + full_path().string_without_scheme() + " has no associated data");
  CTable<Real>::Ptr result = data->follow()->as_type<CTable<Real> >();
  cf_assert( is_not_null(result) );
  return *result;
}

const CF::Mesh::CTable< Real >& CField::data_table() const
{
  Component::ConstPtr data = find_component_ptr_with_tag(*this, "field_data");
  if(!data)
    throw ValueNotFound(FromHere(), "Field " + full_path().string_without_scheme() + " has no associated data");
  CTable<Real>::ConstPtr result = data->follow()->as_type<CTable<Real> const>();
  
  cf_assert( is_not_null(result) );
  return *result;
}

} // Mesh
} // CF
