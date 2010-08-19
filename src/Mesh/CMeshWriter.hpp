#ifndef CF_Mesh_CMeshWriter_hpp
#define CF_Mesh_CMeshWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/ComponentPredicates.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CElements.hpp"

namespace CF {
namespace Mesh {
  
  class CArray;
  
////////////////////////////////////////////////////////////////////////////////

/// CMeshWriter component class
/// This class serves as a component that that will write
/// the mesh to a file
/// @author Willem Deconinck
class Mesh_API CMeshWriter : public Common::Component {

public: // typedefs

  /// provider
  typedef Common::ConcreteProvider < CMeshWriter,1 > PROVIDER;

  /// pointer to this type
  typedef boost::shared_ptr<CMeshWriter> Ptr;
  typedef boost::shared_ptr<CMeshWriter const> ConstPtr;

public: // functions

  /// Contructor
  /// @param name of the component
  CMeshWriter ( const CName& name );

  /// Virtual destructor
  virtual ~CMeshWriter();

  /// Get the class name
  static std::string type_name () { return "CMeshWriter"; }

  /// Configuration Options

  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options );

  // --------- Signals ---------

  void write( Common::XmlNode& node  );

  // --------- Direct access ---------

  virtual std::string get_format() = 0;

  virtual std::vector<std::string> get_extensions() = 0;

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path) = 0;

  boost::filesystem::path write_from(const CMesh::Ptr& mesh);

protected: // classes
  
  class IsGroup
  {
   public:
     IsGroup () {}

     bool operator()(const Component& component)
     {
       return count(range_typed<CElements>(component));
     }
     
  }; // IsGroup

protected:
  
  CMesh::Ptr m_mesh;
  
  typedef std::map<CArray*,std::list<CElements*> > CoordinatesElementsMap;
  CoordinatesElementsMap m_all_coordinates;
  Uint m_coord_dim;
  Uint m_max_dimensionality;
  
  void compute_mesh_specifics();

private: // helper functions

  /// regists all the signals declared in this class
  static void regist_signals ( Component* self ) {}

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshWriter_hpp
