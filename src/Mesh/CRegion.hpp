#ifndef CF_Mesh_CRegion_HH
#define CF_Mesh_CRegion_HH

////////////////////////////////////////////////////////////////////////////////

#include "Common/Component.hpp"
#include "Mesh/MeshAPI.hpp"

namespace CF {
namespace Mesh {

  class CRegion;

////////////////////////////////////////////////////////////////////////////////

  /// Mesh component class
  /// @author Tiago Quintino
  class Mesh_API CRegion : public Common::Component {

  public:

    /// Contructor
    /// @param name of the component
    CRegion ( const CName& name );

    /// Virtual destructor
    virtual ~CRegion();

    /// Get the class name
    static std::string getClassName () { return "CRegion"; }

    // functions specific to the CRegion component

    /// create a region
    /// @param name of the region
    void create_region ( const CName& name );

  private:

    std::vector< boost::shared_ptr<CRegion> > m_subregions;

  };

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CRegion_HH
