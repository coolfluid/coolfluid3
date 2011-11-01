// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionArray.hpp"
#include "common/Foreach.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"

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
  options().add_option< OptionArrayT < URI > > ("Fields", dummy0)
      ->description("Fields to cleanup")
      ->attach_trigger ( boost::bind ( &Reset::config_fields,   this ) );

  std::vector< std::string > dummy1;
  options().add_option( OptionArrayT<std::string>::create("FieldTags", dummy1))
      ->description("Tags of the field for which to apply the action");

  // call config field_tags when mesh is configured

  option("mesh").attach_trigger( boost::bind ( &Reset::config_field_tags,   this ) );
}



void Reset::config_fields()
{
  std::vector<URI> vec; option("Fields").put_value(vec);

  boost_foreach(const URI field_path, vec)
  {
    Component& comp = access_component(field_path);

    if ( Field::Ptr field = comp.as_ptr<Field>() )
    {
      boost::weak_ptr<Field> wptr = field;
      m_fields.push_back( wptr );
    }
    else
      throw ValueNotFound ( FromHere(), "Could not find field with path [" + field_path.path() +"]" );
  }
}


void Reset::config_field_tags()
{
  std::vector<std::string> vec; option("FieldTags").put_value(vec);

  RDSolver& mysolver = solver().as_type<RDSolver>();

  boost_foreach(const std::string tag, vec)
    boost_foreach( Link& link, find_components_with_tag<Link>( mysolver.fields(), tag ) )
    {
      if( Field::Ptr field = link.follow()->as_ptr<Field>() )
      {
        boost::weak_ptr<Field> wptr = field;
        m_fields.push_back( wptr );
      }
    }
}



void Reset::execute()
{
  // loop over fields to cleanup
  boost_foreach(boost::weak_ptr<Field> ptr, m_fields)
  {
    if( ptr.expired() ) continue; // skip if pointer invalid

    Field& field = *ptr.lock();

    field = 0.; // set all entries to zero
  }
}

////////////////////////////////////////////////////////////////////////////////


} // RDM
} // cf3
