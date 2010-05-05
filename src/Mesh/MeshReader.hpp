#ifndef CF_Mesh_MeshReader_hpp
#define CF_Mesh_MeshReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Common/StringOps.hpp"
#include "Common/BasicExceptions.hpp"
#include "Common/ConcreteProvider.hpp"

#include "Mesh/MeshAPI.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {

  using namespace Common;

////////////////////////////////////////////////////////////////////////////////

/// This class represents the the data related to an MeshReader
/// @author Willem Deconinck
class Mesh_API MeshReader {

public: // functions

  typedef Common::ConcreteProvider < MeshReader > PROVIDER;

  /// Default constructor without arguments
  MeshReader();

  /// Default destructor
  ~MeshReader();

  static std::string getClassName() { return "MeshReader"; }

public: // accessors

 
protected: // data


}; // end of class MeshReader
  
////////////////////////////////////////////////////////////////////////////////

} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_MeshReader_hpp
