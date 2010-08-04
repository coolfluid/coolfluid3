#ifndef CF_Mesh_CMeshReader_hpp
#define CF_Mesh_CMeshReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/Component.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CArray.hpp"

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

public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CMeshReader,1 > PROVIDER;
  /// pointer to this type
  typedef boost::shared_ptr<CMeshReader> Ptr;
  typedef boost::shared_ptr<CMeshReader const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMeshReader ( const CName& name );

  /// Virtual destructor
  virtual ~CMeshReader();

  /// Get the class name
  static std::string type_name () { return "CMeshReader"; }
  
  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options );

  // --------- Signals ---------

  void read( Common::XmlNode& node  );

  // --------- Direct access ---------

  virtual std::string get_format() = 0;

  virtual std::vector<std::string> get_extensions() = 0;

  virtual void read_from_to(boost::filesystem::path& path, const CMesh::Ptr& mesh) = 0;

  CMesh::Ptr create_mesh_from(boost::filesystem::path& path);

protected: // functions

  /// Map type from string to a CTable::Buffer
  typedef std::map<std::string,boost::shared_ptr<CTable::Buffer> > BufferMap;

  /// Create element regions for each given type inside a given region
  /// @param [in] parent_region   Region in which the elementregions will be made
  /// @param [in] etypes          List of element type names that will be used
  /// @return a BufferMap with key an etype name and value a buffer for the region
  ///         with name of the etype
  BufferMap create_element_regions_with_buffermap(CRegion& parent_region, const CArray::ConstPtr coordinates,
                                              const std::vector<std::string>& etypes);

  /// remove all regions with empty connectivity tables inside a given region
  /// @param [in] parent_region  Region in which the removal will take place
  void remove_empty_element_regions(CRegion& parent_region);

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshReader_hpp
