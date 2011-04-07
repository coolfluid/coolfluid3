// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/assign/list_of.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/regex.hpp>

#include "Common/Signal.hpp"
#include "Common/CBuilder.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"
#include "Common/CLink.hpp"
#include "Common/FindComponents.hpp"

#include "Mesh/LibMesh.hpp"
#include "Mesh/CField.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CNodes.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CList.hpp"
#include "Mesh/CFieldView.hpp"
#include "Mesh/CCells.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace boost::assign;

Common::ComponentBuilder < CField, Component, LibMesh >  CField_Builder;

////////////////////////////////////////////////////////////////////////////////

CField::Basis::Convert::Convert()
{
  all_fwd = boost::assign::map_list_of
      ( CField::Basis::POINT_BASED, "PointBased" )
      ( CField::Basis::ELEMENT_BASED, "ElementBased" )
      ( CField::Basis::CELL_BASED, "CellBased" )
      ( CField::Basis::FACE_BASED, "FaceBased" );

  all_rev = boost::assign::map_list_of
      ("PointBased",    CField::Basis::POINT_BASED )
      ("ElementBased",  CField::Basis::ELEMENT_BASED )
      ("CellBased",     CField::Basis::CELL_BASED )
      ("FaceBased",     CField::Basis::FACE_BASED );
}

CField::Basis::Convert& CField::Basis::Convert::instance()
{
  static CField::Basis::Convert instance;
  return instance;
}


CField::CField ( const std::string& name  ) :
  Component ( name ),
  m_basis(Basis::POINT_BASED),
  m_space_idx(0u)
{
  mark_basic();
  
  regist_signal ( "create_data_storage" , "Allocate the data", "Create Storage" )->signal->connect ( boost::bind ( &CField::signal_create_data_storage, this, _1 ) );
  
  Option::Ptr uri_option;
  uri_option = m_properties.add_option<OptionURI>("Topology","The field tree this field will be registered in",URI("cpath:"));
  uri_option->attach_trigger ( boost::bind ( &CField::config_tree,   this ) );
  uri_option->mark_basic();
  
  Option::Ptr option;
  option = m_properties.add_option< OptionT<std::string> >("FieldType", "The type of the field", std::string("PointBased"));
  option->restricted_list() += std::string("ElementBased");  
  option->restricted_list() += std::string("CellBased");  
  option->restricted_list() += std::string("FaceBased");  
  option->attach_trigger ( boost::bind ( &CField::config_field_type,   this ) );
  option->mark_basic();
  
  option = m_properties.add_option< OptionT<Uint> >("Space", "The type of the field", 0u);
  option->link_to(&m_space_idx);
  option->mark_basic();
  
  std::vector<std::string> var_names;
  var_names.push_back(name);
  option = m_properties.add_option<OptionArrayT<std::string> >("VarNames","Names of the variables",var_names);
  option->attach_trigger ( boost::bind ( &CField::config_var_names,   this ) );
  option->mark_basic();
  config_var_names();
  
  std::vector<std::string> var_types;
  var_types.push_back("scalar");
  option = m_properties.add_option<OptionArrayT<std::string> >("VarTypes","Types of the variables",var_types);
  m_properties["VarTypes"].as_option().restricted_list() += std::string("scalar") ,
                               std::string("vector2D"),
                               std::string("vector3D"),
                               std::string("tensor2D"),
                               std::string("tensor3D");
  option->attach_trigger ( boost::bind ( &CField::config_var_types,   this ) );
  option->mark_basic();
  config_var_types();

  m_topology = create_static_component<CLink>("topology");
  m_data = create_static_component<CTable<Real> >("data");
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
  boost_foreach (std::string& var_type, var_types)
  {
    if (boost::regex_match(var_type,e_scalar))
    {
      var_type="scalar";
      m_var_types[iVar++]=SCALAR;
    }
    else if (boost::regex_match(var_type,e_vector2d))
    {
      var_type="vector_2D";
      m_var_types[iVar++]=VECTOR_2D;      
    }
    else if (boost::regex_match(var_type,e_vector3d))
    {
      var_type="vector_3D";
      m_var_types[iVar++]=VECTOR_3D;
    }
    else if (boost::regex_match(var_type,e_tensor2d))
    {
      var_type="tensor_2D";
      m_var_types[iVar++]=TENSOR_2D;
    }
    else if (boost::regex_match(var_type,e_tensor3d))
    {
      var_type="tensor_3D";
      m_var_types[iVar++]=TENSOR_3D;      
    }
  }
  // give property a similar look, not all possible regex combinations
  property("VarTypes").change_value(var_types);
}

////////////////////////////////////////////////////////////////////////////////

void CField::config_var_names()
{
  property("VarNames").put_value(m_var_names);
}

////////////////////////////////////////////////////////////////////////////////

void CField::config_field_type()
{
  std::string field_type;
  property("FieldType").put_value(field_type);
  set_basis( Basis::Convert::instance().to_enum(field_type) );
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

void CField::config_tree()
{
  URI topology_uri;
  property("Topology").put_value(topology_uri);
  CRegion::Ptr topology = Core::instance().root()->access_component_ptr(topology_uri)->as_ptr<CRegion>();
  if ( is_null(topology) )
    throw CastingFailed (FromHere(), "Topology must be of a CRegion or derived type");
  m_topology->link_to(topology);
}

////////////////////////////////////////////////////////////////////////////////

void CField::set_topology(CRegion& region)
{
  m_topology->link_to(region.self());
  properties()["Topology"]=region.full_path();
}

////////////////////////////////////////////////////////////////////////////////

CField::~CField()
{
}

////////////////////////////////////////////////////////////////////////////////


void CField::create_data_storage()
{

  cf_assert( m_var_types.size()!=0 );
  cf_assert( is_not_null(m_topology->follow()) );
  
  
  // Check if there are coordinates in this field, and add to map
  m_coords = find_parent_component<CMesh>(topology()).nodes().coordinates().as_ptr<CTable<Real> >();

  Uint row_size(0);
  boost_foreach(const VarType var_size, m_var_types)
    row_size += Uint(var_size);
  
  m_data->set_row_size(row_size);
  
  switch (m_basis)
  {
    case Basis::POINT_BASED:
    {
      m_used_nodes = CElements::used_nodes(topology()).as_ptr<CList<Uint> >();
      m_data->resize(m_used_nodes->size());
      break;
    }
    case Basis::ELEMENT_BASED:
    {
      Uint data_size = 0;
      boost_foreach(CEntities& field_elements, find_components_recursively<CEntities>(topology()))
      {
        if (m_space_idx == 0 && ! field_elements.exists_space(m_space_idx) )
          field_elements.create_space0();
        cf_assert( field_elements.exists_space(m_space_idx) );
        m_elements_start_idx[&field_elements] = data_size;
        CFieldView field_view("tmp_field_view");
        data_size = field_view.initialize(*this,field_elements.as_ptr<CEntities>());
      }
      m_data->resize(data_size);
      break;
    }
    case Basis::CELL_BASED:
    {
      Uint data_size = 0;
      boost_foreach(CEntities& field_elements, find_components_recursively<CCells>(topology()))
      {
        //CFinfo << name() << ": creating cellbased field storage in " << field_elements.full_path().path() << CFendl;
        if (m_space_idx == 0 && ! field_elements.exists_space(m_space_idx) )
          field_elements.create_space0();
        cf_assert( field_elements.exists_space(m_space_idx) );
        m_elements_start_idx[&field_elements] = data_size;
        CFieldView field_view("tmp_field_view");
        data_size = field_view.initialize(*this,field_elements.as_ptr<CEntities>());
      }
      m_data->resize(data_size);
      break;
    }
    case Basis::FACE_BASED:
    {
      Uint data_size = 0;
      boost_foreach(CEntities& field_elements, find_components_recursively_with_tag<CEntities>(topology(),Mesh::Tags::face_entity()))
      {
        if (m_space_idx == 0 && ! field_elements.exists_space(m_space_idx) )
          field_elements.create_space0();
        cf_assert( field_elements.exists_space(m_space_idx) );
        m_elements_start_idx[&field_elements] = data_size;
        CFieldView field_view("tmp_field_view");
        data_size = field_view.initialize(*this,field_elements.as_ptr<CEntities>());
      }
      m_data->resize(data_size);
      break;
    }

    default:
      throw NotSupported(FromHere() , "Basis can only be ELEMENT_BASED or NODE_BASED");
      break;
  }
}

////////////////////////////////////////////////////////////////////////////////

bool CField::has_variable(const std::string& vname) const
{
  return std::find(m_var_names.begin(), m_var_names.end(), vname) != m_var_names.end();
}

////////////////////////////////////////////////////////////////////////////////

Uint CField::var_number ( const std::string& vname ) const
{
  const std::vector<std::string>::const_iterator var_loc_it = std::find(m_var_names.begin(), m_var_names.end(), vname);
  if(var_loc_it == m_var_names.end())
    throw Common::ValueNotFound(FromHere(), "Variable " + vname + " was not found in field " + name());
  return var_loc_it - m_var_names.begin();
}
//////////////////////////////////////////////////////////////////////////////

Uint CField::var_index ( const std::string& vname ) const
{
  const Uint var_nb = var_number(vname);
  return var_index(var_nb);
}

////////////////////////////////////////////////////////////////////////////////

Uint CField::var_index ( const Uint var_nb ) const
{
  Uint var_start = 0;
  for(Uint i = 0; i != var_nb; ++i)
    var_start += m_var_types[i];
  return var_start;
}

//////////////////////////////////////////////////////////////////////////////

CField::VarType CField::var_type ( const std::string& vname ) const
{
  return var_type(var_number(vname));
}

////////////////////////////////////////////////////////////////////////////////

const CRegion& CField::topology() const
{
  return *m_topology->follow()->as_ptr<CRegion>();
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CField::topology()
{
  return *m_topology->follow()->as_ptr<CRegion>();
}

////////////////////////////////////////////////////////////////////////////////

const CList<Uint>& CField::used_nodes() const
{
  return *m_used_nodes;
}

//////////////////////////////////////////////////////////////////////////////  

CTable<Real>::ConstRow CField::coords(const Uint idx) const
{
  return (*m_coords)[used_nodes()[idx]];
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
