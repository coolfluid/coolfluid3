#ifndef CF_Mesh_CMeshWriter_hpp
#define CF_Mesh_CMeshWriter_hpp

////////////////////////////////////////////////////////////////////////////////

#include <boost/filesystem/path.hpp>

#include "Common/ComponentPredicates.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Mesh/MeshAPI.hpp"
#include "Mesh/CMesh.hpp"
#include "Mesh/CTable.hpp"
#include "Mesh/CElements.hpp"
#include "Mesh/CRegion.hpp"

namespace CF {
namespace Mesh {
  
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

public: // static functions

  static CMeshWriter::Ptr create_concrete(const std::string& provider_name, const CName& name)
  {
    Common::SafePtr< CMeshWriter::PROVIDER > prov =
        Common::Factory<CMeshWriter>::getInstance().getProvider(provider_name);
    return boost::dynamic_pointer_cast<CMeshWriter>(prov->create(name));
  }

public: // functions

  /// Contructor
  /// @param name of the component
  CMeshWriter ( const CName& name );

  /// Virtual destructor
  virtual ~CMeshWriter();

  /// Get the class name
  static std::string getClassName () { return "CMeshWriter"; }

  /// Configuration Options

  // --------- Configuration ---------

  static void defineConfigOptions ( Common::OptionList& options );

  // --------- Signals ---------

  void write( Common::XmlNode& node  );

  // --------- Direct access ---------

  virtual std::string get_format() = 0;

  virtual void write_from_to(const CMesh::Ptr& mesh, boost::filesystem::path& path) = 0;

  boost::filesystem::path write_from(const CMesh::Ptr& mesh);

protected: // classes

  class IsLeafRegion
  {
   public:
      IsLeafRegion () {}

      bool operator()(const Component::Ptr& component)
      {
        return component->has_component_of_type<CTable>() && component->has_component_of_type<CElements>();
      }

  };

  class IsGroup
  {
   private:
     IsLeafRegion m_isLeaf;
   public:
     IsGroup () {}

     bool operator()(const Component::Ptr& component)
     {
       BOOST_FOREACH(const CRegion::Ptr& region, component->get_components_by_type<CRegion>())
       {
         if (m_isLeaf(region))
           return true;
       }
       return false;
     }
  };

};

////////////////////////////////////////////////////////////////////////////////

} // Mesh
} // CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_CMeshWriter_hpp
