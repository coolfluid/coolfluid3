// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/OptionArray.hpp"
#include "Common/Foreach.hpp"

#include "Mesh/CField.hpp"
#include "Mesh/CMesh.hpp"

#include "RDM/Core/Cleanup.hpp"

/////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace RDM {

///////////////////////////////////////////////////////////////////////////////////////

Common::ComponentBuilder < Cleanup, CAction, LibCore > Cleanup_Builder;

///////////////////////////////////////////////////////////////////////////////////////
  
Cleanup::Cleanup ( const std::string& name ) : RDM::Action(name)
{
  mark_basic();

  std::vector< URI > dummy;
  m_properties.add_option< OptionArrayT < URI > > ("Fields", "Fields to cleanup", dummy)
      ->attach_trigger ( boost::bind ( &Cleanup::config_fields,   this ) );
}

////////////////////////////////////////////////////////////////////////////////

void Cleanup::config_fields()
{
  std::vector<URI> vec; property("Fields").put_value(vec);

  boost_foreach(const URI field_path, vec)
  {
    Component& comp = access_component(field_path);

    if ( CField::Ptr field = comp.as_ptr<CField>() )
    {
      boost::weak_ptr<CField> wptr = field;
      m_fields.push_back( wptr );
    }
    else
      throw ValueNotFound ( FromHere(), "Could not find field with path [" + field_path.path() +"]" );
  }
}

////////////////////////////////////////////////////////////////////////////////

void Cleanup::execute()
{
  // loop over fields to cleanup
  boost_foreach(boost::weak_ptr<CField> ptr, m_fields)
  {
    if( ptr.expired() ) continue; // skip if pointer invalid

    CTable<Real>& field = ptr.lock()->data();

    field = 0.; // set all entries to zero
  }
}

////////////////////////////////////////////////////////////////////////////////

} // RDM
} // CF

////////////////////////////////////////////////////////////////////////////////////

