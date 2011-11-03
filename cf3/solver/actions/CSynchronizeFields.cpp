// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include <boost/bind.hpp>
#include <boost/function.hpp>

#include "common/Builder.hpp"
#include "common/OptionArray.hpp"
#include "common/Foreach.hpp"

#include "mesh/Field.hpp"
#include "mesh/Mesh.hpp"

#include "CSynchronizeFields.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace solver {
namespace actions {

///////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CSynchronizeFields, common::Action, LibActions > CSynchronizeFields_Builder;

///////////////////////////////////////////////////////////////////////////////////////

CSynchronizeFields::CSynchronizeFields ( const std::string& name ) : solver::Action(name)
{
  mark_basic();

  std::vector< URI > dummy;
  options().add_option< OptionArrayT < URI > > ("Fields", dummy)
      ->description("Fields to synchronize")
      ->attach_trigger ( boost::bind ( &CSynchronizeFields::config_fields,   this ) );
}



void CSynchronizeFields::config_fields()
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



void CSynchronizeFields::execute()
{
  boost_foreach(boost::weak_ptr<Field> ptr, m_fields)
  {
    if( ptr.expired() ) continue; // skip if pointer invalid

    ptr.lock()->synchronize();
  }
}

////////////////////////////////////////////////////////////////////////////////

} // actions
} // solver
} // cf3
