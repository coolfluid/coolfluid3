// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/regex.hpp>

#include "Common/ObjectProvider.hpp"
#include "Common/CLink.hpp"
#include "Common/ComponentPredicates.hpp"

#include "Mesh/MeshAPI.hpp"

#include "Mesh/CMesh.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/ElementType.hpp"

namespace CF {
namespace Mesh {

using namespace Common;

Common::ObjectProvider < CMesh, Component, MeshLib, NB_ARGS_1 >
CMesh_Provider ( CMesh::type_name() );

////////////////////////////////////////////////////////////////////////////////

CMesh::CMesh ( const CName& name  ) :
  Component ( name )
{
  BUILD_COMPONENT;
}

////////////////////////////////////////////////////////////////////////////////

CMesh::~CMesh()
{
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CMesh::create_region( const CName& name )
{
  CRegion::Ptr new_region;
  
  
  if ( range_typed<CRegion>(*this).empty() )
  {
    new_region = create_component_type<CRegion>(name);
    new_region->add_tag("grid_zone");
  }
  else
  {
    CRegion& existing_region = get_component_typed<CRegion>(*this);
    if (existing_region.has_tag("grid_base"))
    {
      //count howmany times the name "name(_[0-9]+)?" occurs (REGEX)
      Uint count = 0;
      boost::regex e(name+"(_[0-9]+)?");
      
      BOOST_FOREACH(const CRegion& region, range_typed<CRegion>(existing_region))
        if (boost::regex_match(region.name(), e))
          count++;

      std::string append = (count == 0) ? "" : "_"+String::to_str(count);
      new_region = existing_region.create_region(name+append).get_type<CRegion>();
      new_region->add_tag("grid_zone");
    }
    else if (existing_region.has_tag("grid_zone"))
    {
      // Create a parent region "base" for the existing region
      CRegion::Ptr base_region = create_component_type<CRegion>("base");
      base_region->add_tag("grid_base");
      existing_region.move_component(base_region);
            
      //count howmany times the name "name(_[0-9]+)?" occurs (REGEX)
      Uint count = 0;
      boost::regex e(name+"(_[0-9]+)?");
      
      BOOST_FOREACH(const CRegion& region, range_typed<CRegion>(*base_region))
      if (boost::regex_match(region.name(), e))
        count++;

      std::string append = (count == 0) ? "" : "_"+String::to_str(count);
      new_region = base_region->create_region(name+append).get_type<CRegion>();
      new_region->add_tag("grid_zone");
    }
    else
    {
      throw ValueNotFound (FromHere(), "The existing region " + existing_region.full_path().string() + " does not have a tag \"grid_zone\" or \"grid_base\"");
    }

    
  }
  
  CFdebug << "Mesh created region " << new_region->full_path().string() << CFendl;
  return *new_region;
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CMesh::create_domain( const CName& name )
{
  CRegion::Ptr new_region = get_child_type<CRegion>(name);
  if (!new_region)
  {
    new_region = create_component_type<CRegion>(name);
    new_region->add_tag("grid_base");
    CFdebug << "Mesh created domain " << new_region->full_path().string() << CFendl;    
  }
  return *new_region;
}

////////////////////////////////////////////////////////////////////////////////
  
  
CField& CMesh::create_field( const CName& name , CRegion& support, const Uint dimension, const CField::DataBasis basis)
{
  CField& field = *create_component_type<CField>(name);
  field.synchronize_with_region(support);
  field.create_data_storage(dimension,basis);
  return field;
}
  
////////////////////////////////////////////////////////////////////////////////

CField& CMesh::create_field( const CName& name, const Uint dimension, const CField::DataBasis basis )
{
  return create_field(name,domain(),dimension,basis);
}
  
////////////////////////////////////////////////////////////////////////////////

const CRegion& CMesh::domain() const
{
  return get_component_typed<CRegion const>(*this);
}

////////////////////////////////////////////////////////////////////////////////

CRegion& CMesh::domain()
{
  return get_component_typed<CRegion>(*this);
}
  
////////////////////////////////////////////////////////////////////////////////

const CField& CMesh::field(const CName& name) const
{
  return get_named_component_typed<CField const>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////

CField& CMesh::field(const CName& name)
{
  return get_named_component_typed<CField>(*this,name);
}

////////////////////////////////////////////////////////////////////////////////


} // Mesh
} // CF
