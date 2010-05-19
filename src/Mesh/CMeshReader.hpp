#ifndef CF_Mesh_CMeshReader_hpp
#define CF_Mesh_CMeshReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>
#include "Common/Component.hpp"
#include "Common/ConcreteProvider.hpp"
#include "Mesh/MeshAPI.hpp"
#include "Mesh/CTable.hpp"

namespace CF {
namespace Mesh {
  class CMesh;
  class CRegion;
  
////////////////////////////////////////////////////////////////////////////////

/// CMeshReader component class
/// This class serves as a component that that will read
/// the mesh format from file
/// @author Willem Deconinck
class Mesh_API CMeshReader : public Common::Component {

public:

  typedef Common::ConcreteProvider < CMeshReader,1 > PROVIDER;

  static boost::shared_ptr<CMeshReader> create_concrete(const std::string& provider_name, const CName& name)
  {
    Common::SafePtr< CMeshReader::PROVIDER > prov =
        Common::Factory<CMeshReader>::getInstance().getProvider(provider_name);
    return boost::dynamic_pointer_cast<CMeshReader>(prov->create(name));
  }

  /// Contructor
  /// @param name of the component
  CMeshReader ( const CName& name );

  /// Virtual destructor
  virtual ~CMeshReader();

  /// Get the class name
  static std::string getClassName () { return "CMeshReader"; }
  
  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options );

  // --------- Signals ---------

  void read();

  // --------- Direct access ---------

  virtual std::string get_format() = 0;

  virtual void read_from_to(boost::filesystem::path& path, const boost::shared_ptr<CMesh>& mesh) = 0;

  boost::shared_ptr<CMesh> create_mesh_from(boost::filesystem::path& path);

// Helper functions
protected:

  /// Map type from string to a CTable::Buffer
  typedef std::map<std::string,boost::shared_ptr<CTable::Buffer> > BufferMap;

  /// Create leaf regions for each given type inside a given region
  /// @param [in] parent_region   Region in which the leafregions will be made
  /// @param [in] etypes          List of element type names that will be used
  /// @return a BufferMap with key an etype name and value a buffer for the region
  ///         with name of the etype
  BufferMap create_leaf_regions_with_buffermap(boost::shared_ptr<CRegion>& parent_region,
                                              std::vector<std::string>& etypes);

  /// remove all regions with empty connectivity tables inside a given region
  /// @param [in] parent_region  Region in which the removal will take place
  void remove_empty_leaf_regions(boost::shared_ptr<CRegion>& parent_region);


// Data
private:


};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshReader_hpp
