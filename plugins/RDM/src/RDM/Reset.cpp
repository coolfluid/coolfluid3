// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionList.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Connectivity.hpp"

#include "RDM/RDSolver.hpp"

#include "Reset.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace RDM {


///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < Reset, common::Action, LibRDM > Reset_Builder;

///////////////////////////////////////////////////////////////////////////////////////

Reset::Reset ( const std::string& name ) : solver::Action(name)
{
  mark_basic();

  std::vector< URI > dummy0;
  options().add("Fields", dummy0)
      .description("Fields to cleanup")
      .attach_trigger ( boost::bind ( &Reset::config_fields,   this ) );

  std::vector< std::string > dummy1;
  options().add("FieldTags", dummy1)
      .description("Tags of the field for which to apply the action");

  // call config field_tags when mesh is configured

  options().option("mesh").attach_trigger( boost::bind ( &Reset::config_field_tags,   this ) );
}



void Reset::config_fields()
{
  std::vector<URI> vec = options().value< std::vector<URI> >("Fields");

  boost_foreach(const URI field_path, vec)
  {
    Handle<Field> field(access_component(field_path));

    if (is_not_null(field))
    {
      m_fields.push_back( field );
    }
    else
      throw ValueNotFound ( FromHere(), "Could not find field with path [" + field_path.path() +"]" );
  }
}


void Reset::config_field_tags()
{
  std::vector<std::string> vec = options().value< std::vector<std::string> >("FieldTags");

  RDSolver& mysolver = *solver().handle<RDSolver>();

  boost_foreach(const std::string tag, vec)
    boost_foreach( Link& link, find_components_with_tag<Link>( mysolver.fields(), tag ) )
    {
      if( Handle< Field > field = link.follow()->handle<Field>() )
      {
        Handle<Field> wptr = field;
        m_fields.push_back( wptr );
      }
    }
}



void Reset::execute()
{
  // loop over fields to cleanup
  boost_foreach(Handle<Field> ptr, m_fields)
  {
    if( is_null(ptr) ) continue; // skip if pointer invalid

    Field& field = *ptr;

    field = 0.; // set all entries to zero
  }
}

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3
