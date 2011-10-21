// Copyright (C) 2010-2011 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "common/Builder.hpp"
#include "common/OptionT.hpp"
#include "common/OptionComponent.hpp"
#include "common/Foreach.hpp"
#include "common/FindComponents.hpp"

#include "mesh/WriteMesh.hpp"
#include "mesh/Mesh.hpp"
#include "mesh/Field.hpp"

#include "CPeriodicWriteMesh.hpp"


using namespace cf3::common;
using namespace cf3::mesh;

namespace cf3 {
namespace Solver {
namespace Actions {

////////////////////////////////////////////////////////////////////////////////////////////

common::ComponentBuilder < CPeriodicWriteMesh, common::Action, LibActions > CPeriodicWriteMesh_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

CPeriodicWriteMesh::CPeriodicWriteMesh ( const std::string& name ) : Solver::Action(name),
  m_writer( create_static_component<WriteMesh>("MeshWriter") )
{
  mark_basic();

  options().add_option( OptionComponent<Component>::create( "iterator", &m_iterator) )
      ->pretty_name("Iterator Component")
      ->description("The component that stores the \'iteration\'");

  options().add_option< OptionT<Uint> >( "saverate", 0 )
      ->pretty_name("Save Rate")
      ->description("Interval of iterations between saves");

  options().add_option< OptionURI >( "filepath", URI() )
      ->pretty_name("File Path")
      ->description("Path where to save the mesh");
}


void CPeriodicWriteMesh::execute()
{
  if( m_iterator.expired() )
    throw SetupError( FromHere(), "The option 'iterator' was not set in the component " + uri().string() );

  const Uint iteration = boost::any_cast<Uint> ( m_iterator.lock()->property("iteration") );

  const Uint saverate = option("saverate").value<Uint>();

  if (saverate == 0) return;

  if ( iteration % saverate == 0 ) // write mesh
  {
    URI filepath = option("filepath").value<URI>();

    /// @note writes all fields to the mesh

    std::vector<URI> state_fields;
    boost_foreach(const Field& field, find_components_recursively<Field>( mesh() ) )
    {
      state_fields.push_back(field.uri());
    }

    m_writer.write_mesh( mesh(), filepath, state_fields );


  }

}

////////////////////////////////////////////////////////////////////////////////

} // Actions
} // Solver
} // cf3
