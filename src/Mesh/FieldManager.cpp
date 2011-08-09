// Copyright (C) 2010 von Karman Institute for Fluid Dynamics, Belgium
//
// This software is distributed under the terms of the
// GNU Lesser General Public License version 3 (LGPLv3).
// See doc/lgpl.txt and doc/gpl.txt for the license text.

#include "Common/CBuilder.hpp"
#include "Common/CGroup.hpp"
#include "Common/OptionComponent.hpp"
#include "Common/OptionT.hpp"
#include "Common/OptionURI.hpp"
#include "Common/Signal.hpp"
#include "Common/FindComponents.hpp"
#include "Common/EventHandler.hpp"

#include "Mesh/FieldManager.hpp"
#include "Mesh/CMeshTransformer.hpp"
#include "Mesh/LoadMesh.hpp"
#include "Mesh/WriteMesh.hpp"

#include "Common/MPI/PE.hpp"

#include "Common/XML/Protocol.hpp"
#include "Common/XML/SignalOptions.hpp"

namespace CF {
namespace Mesh {

using namespace Common;
using namespace Common::XML;
using namespace Common::mpi;

Common::ComponentBuilder < FieldManager, Component, LibMesh > FieldManager_Builder;

////////////////////////////////////////////////////////////////////////////////////////////

struct FieldManager::Implementation
{
  Implementation(Component& component) :
    m_component(component)
  {
  }

  Component& m_component;
};

////////////////////////////////////////////////////////////////////////////////////////////

FieldManager::FieldManager( const std::string& name  ) :
  Component ( name ),
  m_implementation(new Implementation(*this))
{
}

FieldManager::~FieldManager()
{
}

////////////////////////////////////////////////////////////////////////////////

void FieldManager::signal_create_fields(SignalArgs& node)
{
}

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF
