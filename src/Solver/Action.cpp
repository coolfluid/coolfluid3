// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/URI.hpp"
 

#include "Common/OptionArray.hpp"
#include "Common/OptionComponent.hpp"

#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"

#include "Solver/CPhysicalModel.hpp"
#include "Solver/CTime.hpp"
#include "Solver/Action.hpp"

////////////////////////////////////////////////////////////////////////////////////////////

using namespace CF::Common;
using namespace CF::Mesh;

namespace CF {
namespace Solver {

////////////////////////////////////////////////////////////////////////////////////////////

Action::Action ( const std::string& name ) :
  Common::CAction(name)
{
  mark_basic();

  // options

  m_properties.add_option( OptionComponent<CMesh>::create("mesh","Mesh",
                                                          "Mesh the Discretization Method will be applied to",
                                                          &m_mesh))
    ->mark_basic();

  m_properties.add_option( OptionComponent<CPhysicalModel>::create("physical_model", "Physical Model"
                                                                   "Physical model",
                                                                   &m_physical_model))
    ->mark_basic();

  m_properties.add_option( OptionComponent<CTime>::create("time", "Time"
                                                                   "Time tracking component",
                                                                   &m_time))
    ->mark_basic();

  std::vector< URI > dummy;
  m_properties.add_option< OptionArrayT < URI > > ("regions", "Regions", "Regions this action is applied to", dummy)
      ->attach_trigger ( boost::bind ( &Action::config_regions,   this ) );

}

Action::~Action() {}

////////////////////////////////////////////////////////////////////////////////////////////

CPhysicalModel& Action::physical_model()
{
  CPhysicalModel::Ptr model = m_physical_model.lock();

  if( is_null(model) )
    throw Common::SetupError( FromHere(),
                             "Physical Model not yet set for component "
                             + uri().string() );

  return *model;
}

////////////////////////////////////////////////////////////////////////////////////////////

CTime& Action::time()
{
  CTime::Ptr t = m_time.lock();

  if( is_null(t) )
    throw Common::SetupError( FromHere(),
                             "Time not yet set for component "
                             + uri().string() );

  return *t;
}

////////////////////////////////////////////////////////////////////////////////////////////

CMesh& Action::mesh()
{
  CMesh::Ptr m = m_mesh.lock();

  if( is_null(m) )
    throw Common::SetupError( FromHere(),
                             "Mesh not yet set for component "
                             + uri().string() );

  return *m;
}

////////////////////////////////////////////////////////////////////////////////////////////

ComponentIteratorRange<CRegion> Action::regions()
{
  return ComponentIteratorRange<CRegion>(m_loop_regions);
}

////////////////////////////////////////////////////////////////////////////////////////////

void Action::config_regions()
{
  std::vector<URI> vec; property("regions").put_value(vec);

  boost_foreach(const URI region_path, vec)
  {
    Component& comp = access_component(region_path);

    if ( CRegion::Ptr region = comp.as_ptr<CRegion>() )
      m_loop_regions.push_back( region );
    else
      throw ValueNotFound ( FromHere(),
                           "Could not find region with path [" + region_path.path() +"]" );
  }
}

////////////////////////////////////////////////////////////////////////////////////////////

} // Solver
} // CF

////////////////////////////////////////////////////////////////////////////////////////////
