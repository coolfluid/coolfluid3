#ifndef CF_Mesh_Neu_NeuReader_hpp
#define CF_Mesh_Neu_NeuReader_hpp

////////////////////////////////////////////////////////////////////////////////

#include "Mesh/Neu/NeuAPI.hpp"
#include "Mesh/MeshReader.hpp"

////////////////////////////////////////////////////////////////////////////////

namespace CF {
namespace Mesh {
namespace Neu {

//////////////////////////////////////////////////////////////////////////////

/// This class defines Neutral mesh format reader
/// @author Willem Deconinck
class Neu_API NeuReader : public MeshReader
{
public:
  
  /// constructor
  NeuReader();
  
  /// Gets the Class name
  static std::string getClassName() { return "NeuReader"; }
  
private:
  
  void read_headerData(std::fstream& file);
  
  struct HeaderData
  {
    // NUMNP    Total number of nodal points in the mesh
    // NELEM    Total number of elements in the mesh
    // NGPRS    Number of element groups
    // NBSETS   Number of boundary condition sets
    // NDFCD    Number of coordinate directions (2 or 3)
    // NDFVL    Number of velocity components (2 or 3)
    Uint NUMNP, NELEM, NGRPS, NBSETS, NDFCD, NDFVL;
  } m_headerData;
  
  void read_coordinates(std::fstream& file);
  
  void read_connectivity(std::fstream& file);
  
  virtual void read_impl(std::fstream& file)
  {
    // must be in correct order!
    read_headerData(file);
    read_coordinates(file);
    read_connectivity(file);
  }
  
  
}; // end NeuReader


////////////////////////////////////////////////////////////////////////////////

} // namespace Neu
} // namespace Mesh
} // namespace CF

////////////////////////////////////////////////////////////////////////////////

#endif // CF_Mesh_Neu_NeuReader_hpp
