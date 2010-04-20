#ifndef CF_Mesh_CMesh_HH
#define CF_Mesh_CMesh_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

  class CRegion;

////////////////////////////////////////////////////////////////////////////////

  /// Mesh component class
  /// @author Tiago Quintino
  class Mesh_API CMesh : public Common::Component {

  public:

    /// Contructor
    /// @param name of the component
    CMesh ( const CName& name );

    /// Virtual destructor
    virtual ~CMesh();

    /// Get the class name
    static std::string getClassName () { return "CMesh"; }

    // functions specific to the CMesh component


    /// create a region
    /// @param name of the region
    void create_region ( const CName& name );

  private:

    std::vector< boost::shared_ptr<CRegion> > m_regions;

  };

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMesh_HH
