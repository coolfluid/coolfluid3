#ifndef CF_Mesh_Integrators_IntegrationFunctorBase_hpp
#define CF_Mesh_Integrators_IntegrationFunctorBase_hpp

#include "Common/AssertionManager.hpp"
#include "Common/BasicExceptions.hpp"

#include "Mesh/CArray.hpp"
#include "Mesh/CRegion.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/ElementNodes.hpp"

namespace CF {
namespace Mesh {

class CRegion;

namespace Integrators {

/// Base class for functors used in integration.
struct IntegrationFunctorBase {

  /// Sets up a functor for the given mesh
  IntegrationFunctorBase() {}
  /// Sets up the functor to use the specified region
  void setRegion(const CElements& region) {
    m_region = &region;
    m_coordinates = &region.coordinates();
  }
  /// Sets up the functor to use the specified element (relative to the currently set region)
  void setElement(const Uint element) {
    m_nodes = ConstElementNodeView(*m_coordinates, m_region->connectivity_table().table()[element]);
  }

protected:
  const CElements* m_region;
  const CArray* m_coordinates;
  ConstElementNodeView m_nodes;
};

} // namespace Integrators
} // namespace Mesh
} // namespace CF

#endif /* CF_Mesh_Integrators_IntegrationFunctorBase_hpp */
